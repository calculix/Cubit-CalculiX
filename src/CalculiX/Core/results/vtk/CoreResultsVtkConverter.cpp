#include "CoreResultsVtkConverter.hpp"
#include "CoreResultsVtkElements.hpp"
#include "CoreResultsVtkPipeline.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include <vtkCellArray.h>
#include <vtkAbstractArray.h>
#include <vtkCellData.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataAssembly.h>
#include <vtkDataArray.h>
#include <vtkDataObject.h>
#include <vtkDataSetAttributes.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <limits>
#include <map>
#include <set>
#include <utility>

namespace
{
  using Key = CoreResultsVtkPointKey;
  using Coordinates = ElementIntegration::Coordinates;
  using Poll = std::function<bool(double)>;
  using Association = FeaResults::Association;
  using Kind = CoreResultsVtkGroupKind;

  struct Cell
  {
    int element = -1;
    int type = 0;
    std::string side;
    std::vector<int> nodes;
  };

  struct Domain
  {
    FeaResults::Dataset metadata;
    Kind kind = Kind::Block;
    std::vector<Key> points;
    std::vector<int> elements;
    vtkSmartPointer<vtkUnstructuredGrid> geometry;
    vtkSmartPointer<vtkIdTypeArray> offsets;
    vtkSmartPointer<vtkIdTypeArray> connectivity;
  };

  struct Row
  {
    Key key;
    std::size_t data = 0;
  };

  template<class T>
  const T* find_entity(const std::vector<T>& values, int id)
  {
    const auto found = std::lower_bound(values.begin(), values.end(), id, [](const T& value, int key)
    {
      return value.id < key;
    });
    return found != values.end() && found->id == id ? &*found : nullptr;
  }

  const Row* find_row(const std::vector<Row>& values, const Key& key)
  {
    const auto found = std::lower_bound(values.begin(), values.end(), key, [](const Row& row, const Key& value)
    {
      return row.key < value;
    });
    return found != values.end() && found->key == key ? &*found : nullptr;
  }

  bool fits(std::size_t tuples, std::size_t components, std::string& error)
  {
    const auto maximum = static_cast<std::size_t>(std::numeric_limits<vtkIdType>::max());
    if (components == 0 || components > static_cast<std::size_t>(std::numeric_limits<int>::max()) || tuples > maximum / components || tuples > std::numeric_limits<std::size_t>::max() / sizeof(double) / components)
    {
      error = "VTK array size/component count overflow.";
      return false;
    }
    return true;
  }

  template<class T>
  std::shared_ptr<T> own(vtkSmartPointer<T> value)
  {
    T* pointer = value.GetPointer();
    pointer->Register(nullptr);
    return std::shared_ptr<T>(pointer, [](T* object)
    {
      object->Delete();
    });
  }

  const char* kind_name(Kind kind)
  {
    switch (kind)
    {
      case Kind::Block:
      {
        return "Blocks";
      }
      case Kind::Nodeset:
      {
        return "Nodesets";
      }
      case Kind::Surface:
      {
        return "Surfaces";
      }
      case Kind::DatNodes:
      {
        return "DAT_Nodes";
      }
      case Kind::DatElements:
      {
        return "DAT_Elements";
      }
      case Kind::DatIntegrationPoints:
      {
        return "DAT_IntegrationPoints";
      }
    }
    return "Unknown";
  }

  std::string array_name(const FeaResults::Field& field)
  {
    // Length prefixes make names injective even when user set/field names
    // contain separators. Names remain stable across increments.
    return field.source + "/" + std::to_string(field.set_name.size()) + ":" + field.set_name + "/" + std::to_string(field.name.size()) + ":" + field.name;
  }

  void text(vtkFieldData* data, const char* name, const std::string& value)
  {
    auto array = vtkSmartPointer<vtkStringArray>::New();
    array->SetName(name);
    array->InsertNextValue(value);
    data->AddArray(array);
  }

  void timestamp(vtkDataObject* data, double time)
  {
    auto array = vtkSmartPointer<vtkDoubleArray>::New();
    array->SetName("TimeValue");
    array->SetNumberOfTuples(1);
    array->SetValue(0, time);
    data->GetFieldData()->AddArray(array);
    data->GetInformation()->Set(vtkDataObject::DATA_TIME_STEP(), time);
  }

  bool lookup(const CoreResultsVtkFieldPlan& field, const CoreResultsFrd& frd, const CoreResultsDat& dat, std::vector<Row>& output, std::string& error)
  {
    const bool from_frd = field.field.source == "FRD";
    if (!from_frd && field.field.source != "DAT")
    {
      error = "Unknown field source in VTK conversion plan.";
      return false;
    }
    const auto& data = from_frd ? frd.result_block_data : dat.result_block_data;
    const auto& mappings = from_frd ? frd.result_block_node_data : dat.result_block_c1_data;
    if (field.source_data < 0 || static_cast<std::size_t>(field.source_data) >= data.size() || static_cast<std::size_t>(field.source_data) >= mappings.size() || field.field.components.empty())
    {
      error = "Stale or invalid VTK field plan.";
      return false;
    }
    const auto& values = data[field.source_data];
    std::vector<Row> rows;
    rows.reserve(mappings[field.source_data].size());
    for (const auto& mapping : mappings[field.source_data])
    {
      if (mapping.size() < 2 || mapping[0] <= 0 || mapping[1] < 0 || static_cast<std::size_t>(mapping[1]) >= values.size())
      {
        error = "Invalid VTK source-row mapping.";
        return false;
      }
      const auto& value = values[mapping[1]];
      if (field.component_offset > value.size() || field.field.components.size() > value.size() - field.component_offset)
      {
        error = "VTK source components do not match the plan.";
        return false;
      }
      int ip = 0;
      if (field.field.association == Association::IntegrationPoint)
      {
        if (value.size() < 2 || value[0] != mapping[0] || !std::isfinite(value[1]) || value[1] < 1.0 || value[1] > std::numeric_limits<int>::max() || std::floor(value[1]) != value[1])
        {
          error = "Invalid DAT integration-point identity.";
          return false;
        }
        ip = static_cast<int>(value[1]);
      }
      rows.push_back({{mapping[0], ip}, static_cast<std::size_t>(mapping[1])});
    }
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b)
    {
      return a.key < b.key;
    });
    for (std::size_t i = 1; i < rows.size(); ++i)
    {
      if (rows[i - 1].key == rows[i].key)
      {
        error = "Duplicate source identifiers require an explicit reduction policy for field '" + field.field.name + "'.";
        return false;
      }
    }
    output.swap(rows);
    return true;
  }

  bool domain_cells(const CoreResultsVtkPlan& plan, const CoreResultsDat& dat, const CoreResultsVtkGroup& group, Domain& domain, std::vector<Cell>& cells, std::string& error)
  {
    domain.kind = group.kind;
    domain.metadata.name = group.name;
    domain.metadata.kind = kind_name(group.kind);
    domain.metadata.model_id = group.model_id;
    if (group.kind == Kind::DatIntegrationPoints)
    {
      for (const auto& increment : plan.increments)
      {
        for (const auto& field : increment.fields)
        {
          if (field.field.source != "DAT" || field.field.association != Association::IntegrationPoint || field.field.set_name != group.name)
          {
            continue;
          }
          if (field.source_data < 0 || static_cast<std::size_t>(field.source_data) >= dat.result_block_c1_data.size() || static_cast<std::size_t>(field.source_data) >= dat.result_block_data.size())
          {
            error = "Stale integration-point group plan.";
            return false;
          }
          for (const auto& mapping : dat.result_block_c1_data[field.source_data])
          {
            if (mapping.size() < 2 || mapping[1] < 0 || static_cast<std::size_t>(mapping[1]) >= dat.result_block_data[field.source_data].size())
            {
              error = "Invalid integration-point group row.";
              return false;
            }
            const auto& row = dat.result_block_data[field.source_data][mapping[1]];
            if (row.size() < 2 || !std::isfinite(row[1]) || row[1] < 1 || row[1] > std::numeric_limits<int>::max() || std::floor(row[1]) != row[1])
            {
              error = "Invalid integration-point group identifier.";
              return false;
            }
            domain.points.push_back({mapping[0], static_cast<int>(row[1])});
          }
        }
      }
    }
    else if (group.kind == Kind::Nodeset || group.kind == Kind::DatNodes)
    {
      for (int id : group.ids)
      {
        if (!find_entity(plan.mesh.nodes, id))
        {
          error = "Nodeset references a node absent from the planned mesh.";
          return false;
        }
        domain.points.push_back({id, 0});
      }
    }
    else if (group.kind == Kind::Surface)
    {
      for (const auto& side : group.sides)
      {
        const auto* element = find_entity(plan.mesh.elements, side.element_id);
        CoreResultsVtkElements::Description description;
        Cell cell;
        cell.element = side.element_id;
        cell.side = side.side;
        if (!element || !CoreResultsVtkElements::describe_frd(element->frd_type, description, error) || !CoreResultsVtkElements::surface(description, element->calculix_type, side.side, element->connectivity, cell.type, cell.nodes, error))
        {
          if (error.empty())
          {
            error = "Missing surface parent element.";
          }
          return false;
        }
        cells.push_back(std::move(cell));
      }
    }
    else
    {
      for (int id : group.ids)
      {
        const auto* element = find_entity(plan.mesh.elements, id);
        CoreResultsVtkElements::Description description;
        Cell cell;
        cell.element = id;
        if (!element || !CoreResultsVtkElements::describe_frd(element->frd_type, description, error) || !CoreResultsVtkElements::reorder(description, CoreResultsVtkElements::NodeOrder::CalculiX, CoreResultsVtkElements::NodeOrder::Vtk, element->connectivity, cell.nodes, error))
        {
          if (error.empty())
          {
            error = "Missing domain element.";
          }
          return false;
        }
        cell.type = description.vtk_type;
        cells.push_back(std::move(cell));
      }
    }
    for (const auto& cell : cells)
    {
      for (int node : cell.nodes)
      {
        domain.points.push_back({node, 0});
      }
    }
    std::sort(domain.points.begin(), domain.points.end());
    domain.points.erase(std::unique(domain.points.begin(), domain.points.end()), domain.points.end());
    return true;
  }

  using ShapeCache = std::map<std::string, std::vector<std::vector<double>>>;

  bool prepare_shapes(const CoreResultsVtkPlan& plan, const Domain& domain, ShapeCache& cache, std::string& error)
  {
    if (domain.kind != Kind::DatIntegrationPoints)
    {
      return true;
    }
    for (const auto& key : domain.points)
    {
      const auto* element = find_entity(plan.mesh.elements, key.entity_id);
      if (!element || key.integration_point <= 0)
      {
        error = "Missing integration-point parent element.";
        return false;
      }
      auto found = cache.find(element->calculix_type);
      if (found == cache.end())
      {
        CoreResultsVtkElements::Description description;
        std::vector<ElementIntegration::IntegrationPoint> rule;
        if (!CoreResultsVtkElements::describe(element->calculix_type, description, error) || description.integration_type.empty() || !ElementIntegration::get_rule(description.integration_type, rule))
        {
          error = "Unsupported integration-point formulation '" + element->calculix_type + "'.";
          return false;
        }
        std::vector<std::vector<double>> coefficients(rule.size());
        for (std::size_t i = 0; i < rule.size(); ++i)
        {
          if (!ElementIntegration::shape_functions(description.integration_type, rule[i].coordinates, coefficients[i]) || coefficients[i].size() != element->connectivity.size())
          {
            error = "Invalid integration-point shape coefficients.";
            return false;
          }
        }
        found = cache.emplace(element->calculix_type, std::move(coefficients)).first;
      }
      if (static_cast<std::size_t>(key.integration_point) > found->second.size())
      {
        error = "Integration-point ID is outside the selected rule.";
        return false;
      }
    }
    return true;
  }

  bool geometry(const CoreResultsVtkPlan& plan, Domain& domain, const std::vector<Cell>& cells, const ShapeCache& shapes, CoreResultsVtkPipeline& pipeline, const Poll& poll, std::string& error)
  {
    const bool vertices = domain.kind == Kind::Nodeset || domain.kind == Kind::DatNodes || domain.kind == Kind::DatIntegrationPoints;
    const std::size_t cell_count = vertices ? domain.points.size() : cells.size();
    if (!fits(domain.points.size(), 3, error) || !fits(cell_count, 1, error))
    {
      return false;
    }
    if (cell_count == static_cast<std::size_t>(std::numeric_limits<vtkIdType>::max()) || !fits(cell_count + 1, 1, error))
    {
      error = "VTK cell-offset array size overflow.";
      return false;
    }
    auto coordinates = vtkSmartPointer<vtkDoubleArray>::New();
    coordinates->SetNumberOfComponents(3);
    coordinates->SetNumberOfTuples(static_cast<vtkIdType>(domain.points.size()));
    double* positions = coordinates->GetPointer(0);
    if (!positions && !domain.points.empty())
    {
      error = "Cannot allocate VTK point coordinates.";
      return false;
    }
    const auto& keys = domain.points;
    const bool ip = domain.kind == Kind::DatIntegrationPoints;
    const Poll coordinate_progress = [&](double fraction)
    {
      return poll(0.5 * fraction);
    };
    if (!pipeline.run(keys.size(), 3 * sizeof(double), [&](std::size_t begin, std::size_t end, const std::atomic<bool>& cancelled, std::string& failure)
    {
      for (std::size_t i = begin; i < end; ++i)
      {
        if (cancelled.load(std::memory_order_relaxed))
        {
          return true;
        }
        Coordinates point{};
        if (ip)
        {
          const auto* element = find_entity(plan.mesh.elements, keys[i].entity_id);
          const auto& coefficients = shapes.at(element->calculix_type)[keys[i].integration_point - 1];
          for (std::size_t node = 0; node < coefficients.size(); ++node)
          {
            const auto* source = find_entity(plan.mesh.nodes, element->connectivity[node]);
            if (!source)
            {
              failure = "Missing mesh node during integration-point interpolation.";
              return false;
            }
            for (std::size_t axis = 0; axis < 3; ++axis)
            {
              point[axis] += coefficients[node] * source->coordinates[axis];
            }
          }
        }
        else
        {
          const auto* source = find_entity(plan.mesh.nodes, keys[i].entity_id);
          if (!source)
          {
            failure = "Missing node during VTK geometry conversion.";
            return false;
          }
          point = source->coordinates;
        }
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          if (!std::isfinite(point[axis]))
          {
            failure = "Non-finite VTK point coordinate.";
            return false;
          }
          positions[3 * i + axis] = point[axis];
        }
      }
      return true;
    }, coordinate_progress, error))
    {
      return false;
    }
    coordinates->Modified();
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetData(coordinates);
    domain.geometry = vtkSmartPointer<vtkUnstructuredGrid>::New();
    domain.geometry->SetPoints(points);
    domain.offsets = vtkSmartPointer<vtkIdTypeArray>::New();
    domain.offsets->SetNumberOfValues(static_cast<vtkIdType>(cell_count + 1));
    vtkIdType* offsets = domain.offsets->GetPointer(0);
    if (!offsets)
    {
      error = "Cannot allocate VTK cell offsets.";
      return false;
    }
    offsets[0] = 0;
    for (std::size_t i = 0; i < cell_count; ++i)
    {
      const std::size_t count = vertices ? 1 : cells[i].nodes.size();
      if (count > static_cast<std::size_t>(std::numeric_limits<vtkIdType>::max() - offsets[i]))
      {
        error = "VTK connectivity offset overflow.";
        return false;
      }
      offsets[i + 1] = offsets[i] + static_cast<vtkIdType>(count);
    }
    if (!fits(static_cast<std::size_t>(offsets[cell_count]), 1, error))
    {
      return false;
    }
    domain.connectivity = vtkSmartPointer<vtkIdTypeArray>::New();
    domain.connectivity->SetNumberOfValues(offsets[cell_count]);
    vtkIdType* connectivity = domain.connectivity->GetPointer(0);
    auto types = vtkSmartPointer<vtkUnsignedCharArray>::New();
    types->SetNumberOfValues(static_cast<vtkIdType>(cell_count));
    auto* cell_types = types->GetPointer(0);
    if ((!connectivity && offsets[cell_count] > 0) || (!cell_types && cell_count > 0))
    {
      error = "Cannot allocate VTK connectivity/type arrays.";
      return false;
    }
    domain.elements.resize(cell_count, -1);
    auto original_elements = vtkSmartPointer<vtkTypeInt64Array>::New();
    original_elements->SetName("OriginalElementId");
    original_elements->SetNumberOfTuples(static_cast<vtkIdType>(cell_count));
    auto* element_ids = original_elements->GetPointer(0);
    if (!element_ids && cell_count > 0)
    {
      error = "Cannot allocate original-element IDs.";
      return false;
    }
    auto original_sides = vtkSmartPointer<vtkStringArray>::New();
    original_sides->SetName("OriginalSide");
    const Poll cell_progress = [&](double fraction)
    {
      return poll(0.5 + 0.5 * fraction);
    };
    if (!pipeline.run(cell_count, 20 * sizeof(vtkIdType) + sizeof(int) + 1, [&](std::size_t begin, std::size_t end, const std::atomic<bool>& cancelled, std::string& failure)
    {
      for (std::size_t i = begin; i < end; ++i)
      {
        if (cancelled.load(std::memory_order_relaxed))
        {
          return true;
        }
        if (vertices)
        {
          connectivity[i] = static_cast<vtkIdType>(i);
          cell_types[i] = 1; // VTK_VERTEX
          domain.elements[i] = ip ? keys[i].entity_id : -1;
        }
        else
        {
          domain.elements[i] = cells[i].element;
          cell_types[i] = static_cast<unsigned char>(cells[i].type);
          for (std::size_t j = 0; j < cells[i].nodes.size(); ++j)
          {
            const Key key{cells[i].nodes[j], 0};
            const auto found = std::lower_bound(keys.begin(), keys.end(), key);
            if (found == keys.end() || !(*found == key))
            {
              failure = "Invalid compact VTK connectivity mapping.";
              return false;
            }
            connectivity[offsets[i] + static_cast<vtkIdType>(j)] = static_cast<vtkIdType>(found - keys.begin());
          }
        }
        element_ids[i] = domain.elements[i];
      }
      return true;
    }, cell_progress, error))
    {
      return false;
    }
    original_elements->Modified();
    domain.offsets->Modified();
    domain.connectivity->Modified();
    types->Modified();
    if (domain.kind == Kind::Surface)
    {
      for (const auto& cell : cells)
      {
        original_sides->InsertNextValue(cell.side);
      }
    }
    auto vtk_cells = vtkSmartPointer<vtkCellArray>::New();
    if (!vtk_cells->SetData(static_cast<vtkDataArray*>(domain.offsets.GetPointer()), static_cast<vtkDataArray*>(domain.connectivity.GetPointer())))
    {
      error = "VTK rejected the converted cell arrays.";
      return false;
    }
    domain.geometry->SetCells(types, vtk_cells);
    domain.geometry->GetCellData()->AddArray(original_elements);
    if (domain.kind == Kind::Surface)
    {
      domain.geometry->GetCellData()->AddArray(original_sides);
    }
    auto original = vtkSmartPointer<vtkTypeInt64Array>::New();
    original->SetName(ip ? "OriginalElementId" : "OriginalNodeId");
    original->SetNumberOfTuples(static_cast<vtkIdType>(keys.size()));
    auto point_ids = vtkSmartPointer<vtkTypeInt64Array>::New();
    point_ids->SetName("IntegrationPointId");
    if (ip)
    {
      point_ids->SetNumberOfTuples(static_cast<vtkIdType>(keys.size()));
    }
    for (std::size_t i = 0; i < keys.size(); ++i)
    {
      original->SetValue(static_cast<vtkIdType>(i), keys[i].entity_id);
      if (ip)
      {
        point_ids->SetValue(static_cast<vtkIdType>(i), keys[i].integration_point);
      }
    }
    domain.geometry->GetPointData()->AddArray(original);
    if (ip)
    {
      domain.geometry->GetPointData()->AddArray(point_ids);
    }
    text(domain.geometry->GetFieldData(), "GroupName", domain.metadata.name);
    text(domain.geometry->GetFieldData(), "GroupKind", domain.metadata.kind);
    domain.metadata.point_count = keys.size();
    domain.metadata.cell_count = cell_count;
    return true;
  }

  bool applicable(const CoreResultsVtkFieldPlan& field, const Domain& domain)
  {
    if (field.field.source == "FRD")
    {
      return domain.kind != Kind::DatIntegrationPoints;
    }
    if (field.field.set_name != domain.metadata.name)
    {
      return false;
    }
    return (field.field.association == Association::Node && domain.kind == Kind::DatNodes)
        || (field.field.association == Association::Element && domain.kind == Kind::DatElements)
        || (field.field.association == Association::IntegrationPoint && domain.kind == Kind::DatIntegrationPoints);
  }

  bool attach(const CoreResultsVtkPlan& plan, const Domain& domain, vtkUnstructuredGrid* grid, FeaResults::Field metadata, vtkSmartPointer<vtkDoubleArray> array, vtkSmartPointer<vtkUnsignedCharArray> mask, FeaResults::Increment& increment, std::vector<std::string>& diagnostics, std::string& error)
  {
    const vtkIdType count = mask->GetNumberOfTuples();
    bool partial = false;
    for (vtkIdType i = 0; i < count; ++i)
    {
      partial = partial || mask->GetValue(i) == 0;
    }
    if (partial && !plan.options.include_partial)
    {
      diagnostics.push_back("Skipped incomplete field '" + metadata.name + "' in '" + domain.metadata.name + "', step " + std::to_string(increment.step) + ", increment " + std::to_string(increment.increment) + ".");
      return true;
    }
    metadata.dataset_id = domain.metadata.id;
    metadata.array_name = array_name(metadata);
    metadata.validity_array = "Valid/" + metadata.array_name;
    vtkDataSetAttributes* attributes = metadata.association == Association::Element ? static_cast<vtkDataSetAttributes*>(grid->GetCellData()) : static_cast<vtkDataSetAttributes*>(grid->GetPointData());
    if (attributes->HasArray(metadata.array_name.c_str()) || attributes->HasArray(metadata.validity_array.c_str()))
    {
      error = "Ambiguous duplicate field identity '" + metadata.array_name + "'.";
      return false;
    }
    array->SetName(metadata.array_name.c_str());
    mask->SetName(metadata.validity_array.c_str());
    for (std::size_t i = 0; i < metadata.components.size(); ++i)
    {
      array->SetComponentName(static_cast<int>(i), metadata.components[i].c_str());
    }
    array->Modified();
    mask->Modified();
    attributes->AddArray(array);
    attributes->AddArray(mask);
    if (metadata.association != Association::Element && array->GetNumberOfComponents() == 3 && (metadata.name == "DISP" || metadata.interpolated))
    {
      grid->GetPointData()->SetVectors(array);
    }
    increment.fields.push_back(std::move(metadata));
    return true;
  }

  bool field_array(const CoreResultsFrd& frd, const CoreResultsDat& dat, const CoreResultsVtkPlan& plan, const CoreResultsVtkFieldPlan& field, const std::vector<Row>& rows, const Domain& domain, vtkUnstructuredGrid* grid, FeaResults::Increment& increment, std::vector<std::string>& diagnostics, CoreResultsVtkPipeline& pipeline, const Poll& poll, std::string& error)
  {
    const bool cells = field.field.association == Association::Element;
    const std::size_t count = cells ? domain.elements.size() : domain.points.size();
    const std::size_t components = field.field.components.size();
    if (!fits(count, components, error))
    {
      return false;
    }
    const auto& values = field.field.source == "FRD" ? frd.result_block_data[field.source_data] : dat.result_block_data[field.source_data];
    auto array = vtkSmartPointer<vtkDoubleArray>::New();
    array->SetNumberOfComponents(static_cast<int>(components));
    array->SetNumberOfTuples(static_cast<vtkIdType>(count));
    auto mask = vtkSmartPointer<vtkUnsignedCharArray>::New();
    mask->SetNumberOfTuples(static_cast<vtkIdType>(count));
    double* destination = array->GetPointer(0);
    unsigned char* valid = mask->GetPointer(0);
    if ((!destination || !valid) && count > 0)
    {
      error = "Cannot allocate VTK field/validity arrays.";
      return false;
    }
    const double nan = std::numeric_limits<double>::quiet_NaN();
    if (!pipeline.run(count, components * sizeof(double) + sizeof(unsigned char), [&](std::size_t begin, std::size_t end, const std::atomic<bool>& cancelled, std::string& failure)
    {
      (void)failure;
      for (std::size_t i = begin; i < end; ++i)
      {
        if (cancelled.load(std::memory_order_relaxed))
        {
          return true;
        }
        const Key key = cells ? Key{domain.elements[i], 0} : domain.points[i];
        const Row* source = find_row(rows, key);
        bool complete = source != nullptr;
        if (source)
        {
          for (std::size_t c = 0; c < components; ++c)
          {
            complete = complete && std::isfinite(values[source->data][field.component_offset + c]);
          }
        }
        valid[i] = complete ? 1 : 0;
        for (std::size_t c = 0; c < components; ++c)
        {
          destination[i * components + c] = complete ? values[source->data][field.component_offset + c] : nan;
        }
      }
      return true;
    }, poll, error))
    {
      return false;
    }
    return attach(plan, domain, grid, field.field, array, mask, increment, diagnostics, error);
  }

  bool close_time(double a, double b, const CoreResultsVtkOptions& options)
  {
    return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= options.time_absolute_tolerance + options.time_relative_tolerance * std::max(std::abs(a), std::abs(b));
  }

  bool displacement(const CoreResultsFrd& frd, const CoreResultsVtkIncrementPlan& increment, const CoreResultsVtkOptions& options, CoreResultsVtkFieldPlan& field, bool& found, std::string& error)
  {
    found = false;
    for (std::size_t i = 0; i < frd.result_blocks.size(); ++i)
    {
      const auto& block = frd.result_blocks[i];
      if (block.size() < 7 || block[4] < 0 || block[5] < 0 || block[6] < 0 || static_cast<std::size_t>(block[4]) >= frd.total_times.size() || static_cast<std::size_t>(block[5]) >= frd.result_block_type.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_components.size())
      {
        error = "Invalid displacement metadata in the FRD reader.";
        return false;
      }
      if (frd.result_block_type[block[5]] != "DISP" || !close_time(frd.total_times[block[4]], increment.time, options))
      {
        continue;
      }
      if ((increment.step > 0 && block[1] > 0 && increment.step != block[1]) || (increment.increment > 0 && block[2] > 0 && increment.increment != block[2]) || (increment.total_increment > 0 && increment.total_increment != block[3]))
      {
        continue;
      }
      if (found || frd.result_block_components[block[6]].size() != 3)
      {
        error = "Ambiguous or unsupported FRD displacement layout for integration-point interpolation.";
        return false;
      }
      field.source_block = static_cast<int>(i);
      field.source_data = block[6];
      field.component_offset = 0;
      field.field.name = "DISP";
      field.field.source = "FRD";
      field.field.association = Association::Node;
      field.field.components = frd.result_block_components[block[6]];
      found = true;
    }
    return true;
  }

  bool ip_displacement(const CoreResultsFrd& frd, const CoreResultsVtkPlan& plan, const CoreResultsVtkFieldPlan& field, const std::vector<Row>& rows, const ShapeCache& shapes, const Domain& domain, vtkUnstructuredGrid* grid, FeaResults::Increment& increment, std::vector<std::string>& diagnostics, CoreResultsVtkPipeline& pipeline, const Poll& poll, std::string& error)
  {
    const std::size_t count = domain.points.size();
    if (!fits(count, 3, error))
    {
      return false;
    }
    const auto& values = frd.result_block_data[field.source_data];
    auto array = vtkSmartPointer<vtkDoubleArray>::New();
    array->SetNumberOfComponents(3);
    array->SetNumberOfTuples(static_cast<vtkIdType>(count));
    auto mask = vtkSmartPointer<vtkUnsignedCharArray>::New();
    mask->SetNumberOfTuples(static_cast<vtkIdType>(count));
    double* destination = array->GetPointer(0);
    auto* valid = mask->GetPointer(0);
    if ((!destination || !valid) && count > 0)
    {
      error = "Cannot allocate integration-point displacement arrays.";
      return false;
    }
    const double nan = std::numeric_limits<double>::quiet_NaN();
    if (!pipeline.run(count, 3 * sizeof(double) + sizeof(unsigned char), [&](std::size_t begin, std::size_t end, const std::atomic<bool>& cancelled, std::string& failure)
    {
      (void)failure;
      for (std::size_t i = begin; i < end; ++i)
      {
        if (cancelled.load(std::memory_order_relaxed))
        {
          return true;
        }
        const auto& key = domain.points[i];
        const auto* element = find_entity(plan.mesh.elements, key.entity_id);
        const auto& coefficients = shapes.at(element->calculix_type)[key.integration_point - 1];
        Coordinates result{};
        bool complete = true;
        for (std::size_t node = 0; node < coefficients.size(); ++node)
        {
          if (coefficients[node] == 0.0)
          {
            continue;
          }
          const auto* source = find_row(rows, {element->connectivity[node], 0});
          if (!source)
          {
            complete = false;
            break;
          }
          for (std::size_t c = 0; c < 3; ++c)
          {
            const double value = values[source->data][c];
            complete = complete && std::isfinite(value);
            result[c] += coefficients[node] * value;
          }
        }
        complete = complete && std::isfinite(result[0]) && std::isfinite(result[1]) && std::isfinite(result[2]);
        valid[i] = complete ? 1 : 0;
        for (std::size_t c = 0; c < 3; ++c)
        {
          destination[3 * i + c] = complete ? result[c] : nan;
        }
      }
      return true;
    }, poll, error))
    {
      return false;
    }
    FeaResults::Field metadata = field.field;
    metadata.name = "IntegrationPointDisplacement";
    metadata.set_name = domain.metadata.name;
    metadata.association = Association::IntegrationPoint;
    metadata.interpolated = true;
    return attach(plan, domain, grid, metadata, array, mask, increment, diagnostics, error);
  }

  bool partial_cells(const Domain& domain, vtkUnstructuredGrid* grid, const FeaResults::Increment& increment, CoreResultsVtkPipeline& pipeline, const Poll& poll, std::string& error)
  {
    struct Mask
    {
      bool cells = false;
      const unsigned char* values = nullptr;
    };
    std::vector<Mask> masks;
    for (const auto& field : increment.fields)
    {
      if (field.dataset_id != domain.metadata.id)
      {
        continue;
      }
      const bool cells = field.association == Association::Element;
      vtkDataSetAttributes* attributes = cells ? static_cast<vtkDataSetAttributes*>(grid->GetCellData()) : static_cast<vtkDataSetAttributes*>(grid->GetPointData());
      auto* mask = vtkUnsignedCharArray::SafeDownCast(attributes->GetArray(field.validity_array.c_str()));
      const std::size_t expected = cells ? domain.elements.size() : domain.points.size();
      if (!mask || mask->GetNumberOfTuples() != static_cast<vtkIdType>(expected))
      {
        error = "Missing/invalid field validity mask during finalization.";
        return false;
      }
      masks.push_back({cells, mask->GetPointer(0)});
    }
    auto partial = vtkSmartPointer<vtkUnsignedCharArray>::New();
    partial->SetName("Partial");
    partial->SetNumberOfTuples(static_cast<vtkIdType>(domain.elements.size()));
    unsigned char* destination = partial->GetPointer(0);
    if (!destination && !domain.elements.empty())
    {
      error = "Cannot allocate the partial-cell array.";
      return false;
    }
    const vtkIdType* offsets = domain.offsets->GetPointer(0);
    const vtkIdType* connectivity = domain.connectivity->GetPointer(0);
    if (!pipeline.run(domain.elements.size(), 1, [&](std::size_t begin, std::size_t end, const std::atomic<bool>& cancelled, std::string& failure)
    {
      (void)failure;
      for (std::size_t i = begin; i < end; ++i)
      {
        if (cancelled.load(std::memory_order_relaxed))
        {
          return true;
        }
        bool missing = false;
        for (const auto& mask : masks)
        {
          if (mask.cells)
          {
            missing = mask.values[i] == 0;
          }
          else
          {
            for (vtkIdType p = offsets[i]; p < offsets[i + 1]; ++p)
            {
              if (mask.values[connectivity[p]] == 0)
              {
                missing = true;
                break;
              }
            }
          }
          if (missing)
          {
            break;
          }
        }
        destination[i] = missing ? 1 : 0;
      }
      return true;
    }, poll, error))
    {
      return false;
    }
    partial->Modified();
    grid->GetCellData()->AddArray(partial);
    return true;
  }

  bool tables(const CoreResultsDat& dat, const CoreResultsVtkPlan& plan, FeaResults::VtkResultsData& result, const Poll& poll, std::string& error)
  {
    for (std::size_t i = 0; i < plan.tables.size(); ++i)
    {
      if (poll && !poll(0.95 + 0.03 * static_cast<double>(i) / std::max<std::size_t>(1, plan.tables.size())))
      {
        return false;
      }
      const auto& source = plan.tables[i];
      const bool buckle = source.kind == CoreResultsVtkTablePlan::Kind::Buckling;
      const std::vector<std::vector<double>>* rows = nullptr;
      std::vector<std::vector<double>> section;
      if (buckle)
      {
        if (source.source_data >= dat.buckle_data.size())
        {
          error = "Stale DAT buckling table plan.";
          return false;
        }
        rows = &dat.buckle_data[source.source_data];
      }
      else
      {
        if (source.source_data >= dat.result_section_data.size())
        {
          error = "Stale DAT section table plan.";
          return false;
        }
        section.push_back(dat.result_section_data[source.source_data]);
        rows = &section;
      }
      auto table = vtkSmartPointer<vtkTable>::New();
      const std::size_t width = rows->empty() ? 0 : rows->front().size();
      if (width != 0 && !fits(rows->size(), width, error))
      {
        return false;
      }
      for (const auto& row : *rows)
      {
        if (row.size() != width)
        {
          error = "Inconsistent DAT table row width.";
          return false;
        }
      }
      for (std::size_t c = 0; c < width; ++c)
      {
        std::string name = "Column" + std::to_string(c);
        if (buckle && width == 2)
        {
          name = c == 0 ? "Mode" : "BucklingFactor";
        }
        else if (!buckle && c < dat.result_section_label.size())
        {
          name = dat.result_section_label[c];
        }
        auto column = vtkSmartPointer<vtkDoubleArray>::New();
        column->SetName(name.c_str());
        column->SetNumberOfTuples(static_cast<vtkIdType>(rows->size()));
        for (std::size_t r = 0; r < rows->size(); ++r)
        {
          column->SetValue(static_cast<vtkIdType>(r), (*rows)[r][c]);
        }
        table->AddColumn(column);
      }
      text(table->GetFieldData(), "Source", "DAT");
      text(table->GetFieldData(), "ResultKind", buckle ? "Buckling" : "Section");
      text(table->GetFieldData(), "SourceDataIndex", std::to_string(source.source_data));
      if (!buckle)
      {
        auto set_names = vtkSmartPointer<vtkStringArray>::New();
        set_names->SetName("SetName");
        std::set<std::string> seen;
        for (const auto& mapping : dat.result_section)
        {
          if (mapping.size() >= 2 && mapping[1] >= 0 && static_cast<std::size_t>(mapping[1]) == source.source_data && mapping[0] >= 0 && static_cast<std::size_t>(mapping[0]) < dat.result_section_set.size())
          {
            const auto& name = dat.result_section_set[mapping[0]];
            if (seen.insert(name).second)
            {
              set_names->InsertNextValue(name);
            }
          }
        }
        table->GetFieldData()->AddArray(set_names);
      }
      result.tables.push_back(own(table));
    }
    return true;
  }
}

bool CoreResultsVtkConverter::build(const CoreResultsFrd& frd, const CoreResultsDat& dat, const CoreResultsVtkPlan& plan, std::atomic<bool>& cancelled, const std::function<bool(double)>& poll, std::shared_ptr<const FeaResults::VtkResultsData>& output, std::string& error) const
{
  error.clear();
  if (!CoreResultsVtkPipeline::valid(plan.options, error))
  {
    return false;
  }
  const auto observe = [&](double fraction)
  {
    if (cancelled.load(std::memory_order_relaxed))
    {
      return false;
    }
    if (poll && !poll(fraction))
    {
      cancelled.store(true, std::memory_order_relaxed);
      return false;
    }
    return true;
  };
  try
  {
    if (!observe(0.0))
    {
      return false;
    }
    auto result = std::make_shared<FeaResults::VtkResultsData>();
    result->job_id = plan.job_id;
    result->run_identity = plan.run_identity;
    result->input_path = plan.input_path;
    result->input_files = plan.input_files;
    result->link_source = plan.link_source;
    result->diagnostics = plan.diagnostics;
    if (plan.job_id <= 0 || plan.mesh.nodes.empty() || plan.mesh.elements.empty())
    {
      error = "Invalid or empty VTK conversion plan.";
      return false;
    }
    std::vector<Domain> domains;
    ShapeCache shapes;
    CoreResultsVtkPipeline pipeline(plan.options, cancelled);
    std::vector<const CoreResultsVtkGroup*> groups;
    CoreResultsVtkGroup base;
    CoreResultsVtkGroup unassigned;
    const bool has_blocks = std::any_of(plan.mesh.groups.begin(), plan.mesh.groups.end(), [](const CoreResultsVtkGroup& group)
    {
      return group.kind == Kind::Block;
    });
    if (!has_blocks)
    {
      base.name = "Mesh";
      base.kind = Kind::Block;
      base.ids.reserve(plan.mesh.elements.size());
      for (const auto& element : plan.mesh.elements)
      {
        base.ids.push_back(element.id);
      }
      groups.push_back(&base);
    }
    for (const auto& group : plan.mesh.groups)
    {
      groups.push_back(&group);
    }
    // Preserve result nodes which have no FE connectivity. With a collection
    // they get a compact vertex dataset; a single-grid result gets vertex cells
    // below. Do not silently lose reference-point output during compaction.
    std::set<int> referenced;
    for (const auto& element : plan.mesh.elements)
    {
      referenced.insert(element.connectivity.begin(), element.connectivity.end());
    }
    for (const auto& group : plan.mesh.groups)
    {
      if (group.kind == Kind::Nodeset || group.kind == Kind::DatNodes)
      {
        referenced.insert(group.ids.begin(), group.ids.end());
      }
    }
    unassigned.kind = Kind::Nodeset;
    unassigned.name = "UnassignedResultNodes";
    for (const auto& node : frd.nodes)
    {
      if (!node.empty() && referenced.count(node[0]) == 0)
      {
        unassigned.ids.push_back(node[0]);
      }
    }
    if (!unassigned.ids.empty() && plan.layout == FeaResults::DatasetLayout::Collection)
    {
      groups.push_back(&unassigned);
    }
    if (groups.size() > std::numeric_limits<unsigned int>::max())
    {
      error = "Too many spatial datasets for a VTK collection.";
      return false;
    }
    domains.reserve(groups.size());
    for (std::size_t i = 0; i < groups.size(); ++i)
    {
      if (!observe(0.02 + 0.23 * static_cast<double>(i) / groups.size()))
      {
        return false;
      }
      Domain domain;
      domain.metadata.id = i;
      std::vector<Cell> cells;
      if (!domain_cells(plan, dat, *groups[i], domain, cells, error) || !prepare_shapes(plan, domain, shapes, error))
      {
        return false;
      }
      if (groups[i] == &base && plan.layout == FeaResults::DatasetLayout::SingleGrid)
      {
        for (int id : unassigned.ids)
        {
          cells.push_back({-1, 1, "", {id}});
          domain.points.push_back({id, 0});
        }
        std::sort(domain.points.begin(), domain.points.end());
        domain.points.erase(std::unique(domain.points.begin(), domain.points.end()), domain.points.end());
      }
      const Poll progress = [&](double fraction)
      {
        return observe(0.02 + 0.23 * (static_cast<double>(i) + fraction) / groups.size());
      };
      if (!geometry(plan, domain, cells, shapes, pipeline, progress, error))
      {
        return false;
      }
      result->datasets.push_back(domain.metadata);
      domains.push_back(std::move(domain));
    }
    result->layout = plan.layout;
    if (result->layout == FeaResults::DatasetLayout::SingleGrid && domains.size() != 1)
    {
      error = "Spatial datasets do not match the planned single-grid layout.";
      return false;
    }
    // A table-only result still retains an undeformed mesh for the viewport.
    std::vector<CoreResultsVtkIncrementPlan> table_frame;
    if (plan.increments.empty())
    {
      table_frame.emplace_back();
    }
    const auto& increments = plan.increments.empty() ? table_frame : plan.increments;
    std::vector<std::vector<vtkSmartPointer<vtkUnstructuredGrid>>> grids(increments.size());
    result->increments.resize(increments.size());
    for (std::size_t i = 0; i < increments.size(); ++i)
    {
      const auto& source = increments[i];
      auto& destination = result->increments[i];
      destination.step = source.step;
      destination.increment = source.increment;
      destination.total_increment = source.total_increment;
      destination.time = source.time;
      for (const auto& domain : domains)
      {
        auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        grid->ShallowCopy(domain.geometry);
        timestamp(grid, source.time);
        grids[i].push_back(grid);
      }
    }
    double operations = 0.0;
    for (const auto& increment : increments)
    {
      operations += static_cast<double>(increment.fields.size()) * domains.size();
    }
    operations = std::max(1.0, operations);
    double operation = 0.0;
    for (std::size_t i = 0; i < increments.size(); ++i)
    {
      for (const auto& field : increments[i].fields)
      {
        std::vector<Row> rows;
        if (!lookup(field, frd, dat, rows, error))
        {
          return false;
        }
        for (std::size_t d = 0; d < domains.size(); ++d)
        {
          const Poll progress = [&](double fraction)
          {
            return observe(0.25 + 0.60 * (operation + fraction) / operations);
          };
          if (!progress(0.0))
          {
            return false;
          }
          if (applicable(field, domains[d]) && !field_array(frd, dat, plan, field, rows, domains[d], grids[i][d], result->increments[i], result->diagnostics, pipeline, progress, error))
          {
            return false;
          }
          operation += 1.0;
        }
      }
    }
    const double displacement_operations = std::max(1.0, static_cast<double>(increments.size()) * domains.size());
    double displacement_operation = 0.0;
    if (plan.options.interpolate_ip_displacement)
    {
      const bool has_ip = std::any_of(domains.begin(), domains.end(), [](const Domain& domain)
      {
        return domain.kind == Kind::DatIntegrationPoints;
      });
      for (std::size_t i = 0; has_ip && i < increments.size(); ++i)
      {
        CoreResultsVtkFieldPlan field;
        bool found = false;
        if (!displacement(frd, increments[i], plan.options, field, found, error))
        {
          return false;
        }
        std::vector<Row> rows;
        if (found && !lookup(field, frd, dat, rows, error))
        {
          return false;
        }
        if (!found)
        {
          result->diagnostics.push_back("No matching FRD displacement for integration-point deformation at step " + std::to_string(increments[i].step) + ", increment " + std::to_string(increments[i].increment) + ". Reference coordinates are retained.");
        }
        for (std::size_t d = 0; d < domains.size(); ++d)
        {
          const Poll progress = [&](double fraction)
          {
            return observe(0.85 + 0.10 * (displacement_operation + fraction) / displacement_operations);
          };
          if (!progress(0.0))
          {
            return false;
          }
          if (found && domains[d].kind == Kind::DatIntegrationPoints && !ip_displacement(frd, plan, field, rows, shapes, domains[d], grids[i][d], result->increments[i], result->diagnostics, pipeline, progress, error))
          {
            return false;
          }
          displacement_operation += 1.0;
        }
      }
    }
    if (!tables(dat, plan, *result, observe, error))
    {
      return false;
    }
    for (std::size_t i = 0; i < result->increments.size(); ++i)
    {
      if (!observe(0.98 + 0.02 * static_cast<double>(i) / std::max<std::size_t>(1, result->increments.size())))
      {
        return false;
      }
      vtkSmartPointer<vtkPartitionedDataSetCollection> collection;
      vtkSmartPointer<vtkDataAssembly> assembly;
      std::map<std::string, int> categories;
      if (result->layout == FeaResults::DatasetLayout::Collection)
      {
        collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
        collection->SetNumberOfPartitionedDataSets(static_cast<unsigned int>(domains.size()));
        assembly = vtkSmartPointer<vtkDataAssembly>::New();
        assembly->SetRootNodeName("Results");
      }
      for (std::size_t d = 0; d < domains.size(); ++d)
      {
        const Poll progress = [&](double fraction)
        {
          return observe(0.98 + 0.02 * (static_cast<double>(i) + (static_cast<double>(d) + fraction) / domains.size()) / result->increments.size());
        };
        if (!partial_cells(domains[d], grids[i][d], result->increments[i], pipeline, progress, error))
        {
          return false;
        }
        auto* grid = grids[i][d].GetPointer();
        for (int association = 0; association < 2; ++association)
        {
          vtkDataSetAttributes* attributes = association == 0 ? static_cast<vtkDataSetAttributes*>(grid->GetPointData()) : static_cast<vtkDataSetAttributes*>(grid->GetCellData());
          const vtkIdType expected = association == 0 ? grid->GetNumberOfPoints() : grid->GetNumberOfCells();
          for (int a = 0; a < attributes->GetNumberOfArrays(); ++a)
          {
            if (attributes->GetAbstractArray(a)->GetNumberOfTuples() != expected)
            {
              error = "VTK field tuple count does not match its geometry.";
              return false;
            }
          }
        }
        if (collection)
        {
          const auto id = static_cast<unsigned int>(d);
          collection->SetPartition(id, 0, grid);
          collection->GetMetaData(id)->Set(vtkCompositeDataSet::NAME(), domains[d].metadata.name.c_str());
          const std::string& category = domains[d].metadata.kind;
          if (categories.count(category) == 0)
          {
            categories[category] = assembly->AddNode(category.c_str());
          }
          const std::string name = "Dataset_" + std::to_string(d);
          const int node = assembly->AddNode(name.c_str(), categories.at(category));
          if (node < 0 || !assembly->AddDataSetIndex(node, id))
          {
            error = "VTK rejected the dataset assembly.";
            return false;
          }
        }
      }
      vtkSmartPointer<vtkDataObject> dataset;
      if (collection)
      {
        collection->SetDataAssembly(assembly);
        timestamp(collection, result->increments[i].time);
        dataset = collection;
      }
      else
      {
        dataset = grids[i][0];
      }
      result->increments[i].dataset = own(dataset);
    }
    pipeline.finish();
    if (!observe(1.0))
    {
      return false;
    }
    output = std::move(result);
    return true;
  }
  catch (const std::exception& failure)
  {
    error = "VTK dataset construction failed: " + std::string(failure.what());
    return false;
  }
  catch (...)
  {
    error = "Unexpected VTK dataset construction failure.";
    return false;
  }
}
