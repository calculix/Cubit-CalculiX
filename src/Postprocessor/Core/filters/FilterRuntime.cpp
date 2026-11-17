#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "FilterRuntime.hpp"
#include "../representations/ResultMesh.hpp"
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkExtractEdges.h>
#include <vtkExtractCells.h>
#include <vtkIdList.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkNew.h>
#include <map>
#include <set>
#include <limits>
#include <cmath>
#include <algorithm>
namespace Postprocessor
{
  std::string filter_coordinate_unit(const SimResults::VtkResultsData& source, std::size_t frame)
  {
    if (frame < source.increments.size() && !source.increments[frame].coordinate_unit.empty())
    {
      return source.increments[frame].coordinate_unit;
    }
    return source.source_units.coordinate_unit;
  }
  namespace
  {
    constexpr const char* parentName = "__CubitSimFilterParentCell";
    template<class T> std::shared_ptr<T> own(T* value) {
      return std::shared_ptr<T>(value,[](T* pointer) {
        pointer->Delete();
      });
    }
    std::shared_ptr<vtkPolyData> original_points(vtkDataSet* mesh,const FilterDefinition& filter) {
      auto result = own(vtkPolyData::New());
      vtkNew<vtkPoints> points;
      points->SetDataTypeToDouble();
      vtkNew<vtkCellArray> vertices;
      result->GetPointData()->CopyAllocate(mesh->GetPointData());
      const bool copyVertexCells = mesh->GetPointData()->HasArray("IntegrationPointId") && mesh->GetNumberOfCells() == mesh->GetNumberOfPoints();
      if (copyVertexCells) {
        result->GetCellData()->CopyAllocate(mesh->GetCellData());
      }
      double bounds[6];
      mesh->GetBounds(bounds);
      const double tolerance = FilterDefaults::relative_tolerance*std::max(1.0,
        std::hypot(bounds[1]-bounds[0],bounds[3]-bounds[2],bounds[5]-bounds[4]));
      for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i) {
        std::array<double,3> point;
        mesh->GetPoint(i,point.data());
        const double d = signed_distance(filter.plane,point);
        bool keep = d <= tolerance;
        if (filter.keep == ClipSide::Positive) {
          keep = d >= -tolerance;
        }
        if (filter.kind == FilterKind::Section) {
          keep = std::abs(d) <= tolerance;
        }
        if (!filter.enabled)
        {
          keep = true;
        }
        if (keep) {
          const auto id = points->InsertNextPoint(point.data());
          vertices->InsertNextCell(1,&id);
          result->GetPointData()->CopyData(mesh->GetPointData(),i,id);
          if (copyVertexCells) {
            result->GetCellData()->CopyData(mesh->GetCellData(), i, id);
          }
        }
      }
      result->SetPoints(points);
      result->SetVerts(vertices);
      return result;
    }
    std::shared_ptr<vtkPolyData> all_edges(vtkDataSet* mesh)
    {
      vtkNew<vtkExtractEdges> edges;
      edges->SetInputData(mesh);
      edges->UseAllPointsOn();
      edges->Update();
      auto output = own(vtkPolyData::New());
      output->ShallowCopy(edges->GetOutput());
      return output;
    }
    std::shared_ptr<vtkPolyData> clip_edges(vtkDataSet* mesh,const FilterDefinition& filter) {
      if (!filter.enabled)
      {
        return all_edges(mesh);
      }
      vtkNew<vtkExtractEdges> edges;
      edges->SetInputData(mesh);
      edges->UseAllPointsOn();
      edges->Update();
      vtkNew<vtkPlane> plane;
      plane->SetOrigin(filter.plane.origin.data());
      plane->SetNormal(filter.plane.normal.data());
      vtkNew<vtkClipPolyData> clip;
      clip->SetInputConnection(edges->GetOutputPort());
      clip->SetClipFunction(plane);
      clip->SetInsideOut(filter.keep == ClipSide::Negative);
      clip->SetValue(0);
      clip->Update();
      auto output = own(vtkPolyData::New());
      output->ShallowCopy(clip->GetOutput());
      return output;
    }
    std::shared_ptr<vtkPolyData> section_edges(vtkDataSet* mesh) {
      // Keep boundaries between source cells; suppress subdivision diagonals
      // shared only by fragments of the same source cell.
      struct Edge
      {
        vtkIdType parent = -1;
        unsigned int uses = 0;
        bool boundary = false;
      };
      std::map<std::pair<vtkIdType,vtkIdType>,Edge> edges;
      auto* parents = vtkIdTypeArray::SafeDownCast(mesh->GetCellData()->GetArray(parentName));
      const auto add = [&](vtkIdType a,vtkIdType b,vtkIdType parent) {
        if (a > b) {
          std::swap(a,b);
        }
        auto& edge = edges[{a,b}];
        edge.boundary = edge.boundary || (edge.uses != 0 && edge.parent != parent);
        edge.parent = parent;
        ++edge.uses;
      };
      for (vtkIdType i = 0; i < mesh->GetNumberOfCells(); ++i) {
        auto* cell = mesh->GetCell(i);
        auto parent = i;
        if (parents) {
          parent = parents->GetValue(i);
        }
        if (cell->GetCellDimension() == 1) {
          for (vtkIdType j = 1; j < cell->GetNumberOfPoints(); ++j) {
            add(cell->GetPointId(j-1),cell->GetPointId(j),parent);
          }
        }
        else {
          for (int j = 0; j < cell->GetNumberOfEdges(); ++j) {
            auto* edge = cell->GetEdge(j);
            add(edge->GetPointId(0),edge->GetPointId(1),parent);
          }
        }
      }
      auto result = own(vtkPolyData::New());
      vtkNew<vtkPoints> points;
      points->SetDataTypeToDouble();
      points->SetNumberOfPoints(mesh->GetNumberOfPoints());
      for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i) {
        double point[3];
        mesh->GetPoint(i,point);
        points->SetPoint(i,point);
      }
      vtkNew<vtkCellArray> lines;
      for (const auto& entry : edges) {
        if (entry.second.uses == 1 || entry.second.boundary) {
          const vtkIdType ids[] = {entry.first.first,entry.first.second};
          lines->InsertNextCell(2,ids);
        }
      }
      result->SetPoints(points);
      result->SetLines(lines);
      return result;
    }
    std::shared_ptr<vtkDataSet> prepare(vtkDataSet* mesh,const SimResults::Increment& frame,std::size_t dataset) {
      auto result = own(mesh->NewInstance());
      result->ShallowCopy(mesh);
      // Integer IDs must never go through point interpolation.
      for (const auto* key : {"OriginalNodeId","OriginalElementId","IntegrationPointId"}) {
        result->GetPointData()->RemoveArray(key);
      }
      vtkNew<vtkIdTypeArray> parents;
      parents->SetName(parentName);
      parents->SetNumberOfValues(mesh->GetNumberOfCells());
      for (vtkIdType i = 0; i < mesh->GetNumberOfCells(); ++i) {
        parents->SetValue(i,i);
      }
      result->GetCellData()->AddArray(parents);
      for (const auto& field : frame.fields) {
        if (field.dataset_id == dataset && field.categorical && field.association != SimResults::Association::Element) {
          result->GetPointData()->RemoveArray(field.array_name.c_str());
          continue;
        }
        if (field.dataset_id != dataset || field.association != SimResults::Association::Node) {
          continue;
        }
        auto* array = result->GetPointData()->GetArray(field.array_name.c_str());
        vtkDataArray* mask = nullptr;
        if (!field.validity_array.empty()) {
          mask = mesh->GetPointData()->GetArray(field.validity_array.c_str());
        }
        if (!array) {
          continue;
        }
        vtkNew<vtkDoubleArray> values;
        values->DeepCopy(array);
        values->SetName(field.array_name.c_str());
        for (vtkIdType i = 0; i < values->GetNumberOfTuples(); ++i) {
          bool valid = field.validity_array.empty() || (mask && i < mask->GetNumberOfTuples() &&
            std::isfinite(mask->GetComponent(i,0)) && mask->GetComponent(i,0) > 0);
          for (int c = 0; c < values->GetNumberOfComponents(); ++c) {
            if (!valid || !std::isfinite(values->GetComponent(i,c))) {
              values->SetComponent(i,c,std::numeric_limits<double>::quiet_NaN());
            }
          }
        }
        result->GetPointData()->AddArray(values);
      }
      return result;
    }
  }
  std::shared_ptr<vtkPolyData> filter_parent_edges(vtkDataSet* mesh,const FilterDefinition& filter,
    const SimResults::Increment& frame,std::size_t dataset,const std::vector<std::size_t>& cells,std::string& error) {
    vtkNew<vtkIdList> ids;
    for (auto index : cells) {
      ids->InsertNextId(static_cast<vtkIdType>(index));
    }
    vtkNew<vtkExtractCells> extract;
    extract->SetInputData(mesh);
    extract->SetCellList(ids);
    extract->Update();
    if (!filter.enabled)
    {
      return all_edges(extract->GetOutput());
    }
    if (filter.kind == FilterKind::Clip) {
      return clip_edges(extract->GetOutput(),filter);
    }
    std::shared_ptr<vtkDataSet> section;
    if (!evaluate_section(prepare(extract->GetOutput(),frame,dataset).get(),filter,section,error)) {
      return {};
    }
    return section_edges(section.get());
  }
  bool evaluate_filter(const FilterDefinition& filter,const Representation& display,
    std::shared_ptr<const SimResults::VtkResultsData> source,EvaluatedOutput& output,std::string& error) {
    error.clear();
    if (!source || display.increment_index >= source->increments.size() || !valid_filter(filter,error)) {
      if (error.empty()) {
        error = "Filter input frame is unavailable.";
      }
      return false;
    }
    const auto coordinateUnit = filter_coordinate_unit(*source,display.increment_index);
    if (!filter.coordinate_unit.empty())
    {
      double scale = 0.0;
      if (coordinateUnit.empty() || !framework_units().convert(1.0,filter.coordinate_unit,coordinateUnit,scale,error) ||
          std::abs(scale-1.0) > 1e-12)
      {
        error = "The source coordinate-unit basis changed; recreate the plane filter using the current native coordinates.";
        return false;
      }
    }
    EvaluatedOutput candidate;
    candidate.source = source;
    candidate.source_id = filter.input.object_id;
    auto data = std::make_shared<SimResults::VtkResultsData>(*source);
    auto& frame = data->increments[display.increment_index];
    auto collection = own(vtkPartitionedDataSetCollection::New());
    for (const auto& input : result_mesh_parts(frame.dataset.get())) {
      FilteredPart part;
      part.dataset = input.first;
      std::shared_ptr<vtkDataSet> moved;
      std::string warning;
      if (!deform_mesh(input.second,input.first,frame,display.deformation,moved,warning,error)) {
        return false;
      }
      if (!warning.empty()) {
        candidate.warnings[input.first] = warning;
      }
      auto* mesh = input.second;
      if (moved) {
        mesh = moved.get();
      }
      const bool ip = mesh->GetPointData()->HasArray("IntegrationPointId");
      part.nodes = original_points(mesh,filter);
      if (!filter.enabled)
      {
        part.mesh = prepare(mesh,frame,input.first);
        part.edges = all_edges(mesh);
        if (display.deformation.enabled && display.deformation.overlay)
        {
          part.reference_edges = all_edges(input.second);
        }
      }
      else if (ip) {
        // Zero-dimensional IP data has no section surface and is never interpolated.
        if (filter.kind == FilterKind::Section) {
          part.nodes = own(vtkPolyData::New());
        }
        part.mesh = part.nodes;
        part.edges = own(vtkPolyData::New());
      }
      else {
        auto prepared = prepare(mesh,frame,input.first);
        bool evaluated = false;
        if (filter.kind == FilterKind::Clip) {
          evaluated = evaluate_clip(prepared.get(), filter, part.mesh, error);
        } else {
          evaluated = evaluate_section(prepared.get(), filter, part.mesh, error);
        }
        if (!evaluated) {
          return false;
        }
        auto* parents = vtkIdTypeArray::SafeDownCast(part.mesh->GetCellData()->GetArray(parentName));
        if (part.mesh->GetNumberOfCells() && (!parents || parents->GetNumberOfTuples() != part.mesh->GetNumberOfCells())) {
          error = "VTK did not preserve filter parent-cell provenance.";
          return false;
        }
        for (vtkIdType i = 0; i < part.mesh->GetNumberOfCells(); ++i) {
          if (parents->GetValue(i) < 0 || parents->GetValue(i) >= mesh->GetNumberOfCells()) {
            error = "VTK returned invalid filter parent-cell provenance.";
            return false;
          }
        }
        if (filter.kind == FilterKind::Clip) {
          if (filter.reference_visible)
          {
            auto opposite = filter;
            opposite.keep = ClipSide::Negative;
            if (filter.keep == ClipSide::Negative)
            {
              opposite.keep = ClipSide::Positive;
            }
            if (!evaluate_clip(prepared.get(),opposite,part.removed,error))
            {
              return false;
            }
          }
          part.edges = clip_edges(prepared.get(), filter);
        } else {
          part.edges = section_edges(part.mesh.get());
        }
        if (display.deformation.enabled && display.deformation.overlay) {
          std::shared_ptr<vtkDataSet> reference;
          if (filter.kind == FilterKind::Clip) {
            part.reference_edges = clip_edges(input.second,filter);
          }
          else if (evaluate_section(prepare(input.second,frame,input.first).get(),filter,reference,error)) {
            part.reference_edges = section_edges(reference.get());
          }
          else {
            return false;
          }
        }
      }
      const auto index = static_cast<unsigned int>(input.first);
      if (collection->GetNumberOfPartitionedDataSets() <= index) {
        collection->SetNumberOfPartitionedDataSets(index+1);
      }
      collection->SetPartition(index,0,part.mesh.get());
      candidate.parts.push_back(std::move(part));
    }
    frame.dataset = collection;
    data->layout = SimResults::DatasetLayout::Collection;
    candidate.data = std::move(data);
    output = std::move(candidate);
    return true;
  }
}
