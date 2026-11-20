#include "TopologyElementPartitioner.hpp"
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkExtractCells.h>
#include <vtkIdList.h>
#include <vtkNew.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVariant.h>
#include <utility>

namespace
{
  template<class T> std::shared_ptr<T> own_vtk()
  {
    return {T::New(),[](T* value) { value->Delete(); }};
  }

  void attach(SimResults::Increment& frame, vtkDataSetAttributes* attributes, vtkDataArray* ids,
    const std::map<long long,const std::vector<double>*>& rows, std::size_t dataset,
    const char* name, std::size_t column, SimResults::Association association)
  {
    if (!ids)
    {
      return;
    }
    SimResults::Field field;
    field.name = name;
    field.source = "Topology";
    field.array_name = std::string("Topology/") + name;
    field.validity_array = "Valid/" + field.array_name;
    field.components = {"Value"};
    field.component_units.emplace_back("", "1", SimUnits::Dimensionless);
    field.dataset_id = dataset;
    field.association = association;
    vtkNew<vtkDoubleArray> values;
    vtkNew<vtkUnsignedCharArray> valid;
    values->SetName(field.array_name.c_str());
    valid->SetName(field.validity_array.c_str());
    for (vtkIdType i = 0; i < ids->GetNumberOfTuples(); ++i)
    {
      const auto found = rows.find(ids->GetVariantValue(i).ToLongLong());
      double value = 0;
      if (found != rows.end())
      {
        value = (*found->second)[column];
      }
      values->InsertNextValue(value);
      valid->InsertNextValue(found != rows.end());
    }
    attributes->AddArray(values);
    attributes->AddArray(valid);
    frame.fields.push_back(std::move(field));
  }
}

bool TopologyElementPartitioner::split(const SimResults::VtkResultsData& child, const SimResults::Increment& frame,
  const std::vector<std::vector<double>>& elements, const std::vector<std::vector<double>>& nodes,
  bool levelSet, double threshold, SimResults::Increment& output,
  std::vector<SimResults::Dataset>& datasets, MeshIdentity& identity, std::string& error)
{
  std::map<long long,const std::vector<double>*> elementRows, nodeRows;
  for (const auto& row : elements)
  {
    elementRows.emplace(static_cast<long long>(row[0]),&row);
  }
  for (const auto& row : nodes)
  {
    nodeRows.emplace(static_cast<long long>(row[0]),&row);
  }
  auto collection = own_vtk<vtkPartitionedDataSetCollection>();
  auto* input = vtkPartitionedDataSetCollection::SafeDownCast(frame.dataset.get());
  identity.clear();
  output = frame;
  output.fields.clear();
  datasets.clear();
  for (const auto& block : child.datasets)
  {
    vtkDataObject* part = frame.dataset.get();
    if (input)
    {
      part = input->GetPartition(static_cast<unsigned int>(block.id),0);
    }
    auto* mesh = vtkUnstructuredGrid::SafeDownCast(part);
    const bool primary = block.kind == "Blocks" || (!input && block.kind.empty());
    if (!mesh && part)
    {
      error = "Topology series requires CalculiX unstructured result grids.";
      return false;
    }
    std::array<std::vector<vtkIdType>,3> cells;
    if (primary && mesh && mesh->GetNumberOfCells())
    {
      auto* ids = mesh->GetCellData()->GetArray("OriginalElementId");
      auto* nodeIds = mesh->GetPointData()->GetArray("OriginalNodeId");
      if (!ids || ids->GetNumberOfTuples() != mesh->GetNumberOfCells() ||
          !nodeIds || nodeIds->GetNumberOfTuples() != mesh->GetNumberOfPoints())
      {
        error = "A topology block has no original element IDs.";
        return false;
      }
      vtkNew<vtkIdList> connectivity;
      for (vtkIdType i = 0; i < mesh->GetNumberOfCells(); ++i)
      {
        bool valid = false;
        const auto id = ids->GetVariantValue(i).ToLongLong(&valid);
        // Single-grid conversion can include auxiliary vertex cells for
        // otherwise unassigned result nodes. They have no element identity.
        if (valid && id == -1 && mesh->GetCellType(i) == VTK_VERTEX)
        {
          cells[2].push_back(i);
          continue;
        }
        if (!valid || id <= 0)
        {
          error = "A topology block has an invalid original element ID.";
          return false;
        }
        const auto row = elementRows.find(id);
        std::size_t group = 2;
        if (row != elementRows.end())
        {
          group = 0;
          if ((*row->second)[3] == 0 && (*row->second)[1] < threshold)
          {
            group = 1;
          }
          std::vector<long long> cellIdentity{static_cast<long long>(block.id),mesh->GetCellType(i)};
          mesh->GetCellPoints(i,connectivity);
          for (vtkIdType j = 0; j < connectivity->GetNumberOfIds(); ++j)
          {
            const auto node = nodeIds->GetVariantValue(connectivity->GetId(j)).ToLongLong(&valid);
            if (!valid || node <= 0)
            {
              error = "A topology element has an invalid original node ID.";
              return false;
            }
            cellIdentity.push_back(node);
          }
          if (!identity.emplace(id,std::move(cellIdentity)).second)
          {
            error = "A topology element appears in multiple primary blocks.";
            return false;
          }
        }
        cells[group].push_back(i);
      }
    }
    const unsigned int count = primary ? 3 : 1;
    for (unsigned int group = 0; group < count; ++group)
    {
      auto item = block;
      item.id = datasets.size();
      item.initially_hidden = true;
      if (primary)
      {
        static const char* names[] = {"Retained", "Removed", "Outside design domain"};
        item.name = std::string(names[group]) + " / " + block.name;
        item.kind = "Blocks";
        item.model_id = -1; // Split siblings must resolve by their distinct names.
        item.initially_hidden = group != 0;
      }
      auto grid = own_vtk<vtkUnstructuredGrid>();
      if (mesh && !primary)
      {
        grid->ShallowCopy(mesh);
      }
      else if (mesh && !cells[group].empty())
      {
        vtkNew<vtkIdList> ids;
        for (const auto id : cells[group])
        {
          ids->InsertNextId(id);
        }
        vtkNew<vtkExtractCells> extract;
        extract->SetInputData(mesh);
        extract->SetCellList(ids);
        extract->Update();
        if (extract->GetOutput()->GetNumberOfCells() != static_cast<vtkIdType>(cells[group].size()))
        {
          error = "VTK could not extract a topology element set.";
          return false;
        }
        grid->ShallowCopy(extract->GetOutput());
      }
      item.point_count = static_cast<std::size_t>(grid->GetNumberOfPoints());
      item.cell_count = static_cast<std::size_t>(grid->GetNumberOfCells());
      for (auto field : frame.fields)
      {
        if (field.dataset_id == block.id && item.point_count)
        {
          field.dataset_id = item.id;
          output.fields.push_back(std::move(field));
        }
      }
      if (primary && group != 2 && item.cell_count)
      {
        auto* attributes = grid->GetCellData();
        auto* ids = attributes->GetArray("OriginalElementId");
        if (!ids)
        {
          error = "VTK extraction lost original topology element IDs.";
          return false;
        }
        attach(output,attributes,ids,elementRows,item.id,levelSet ? "Topology Solid Fraction" : "Topology Density",1,SimResults::Association::Element);
        attach(output,attributes,ids,elementRows,item.id,"Topology Applied Stiffness Factor",2,SimResults::Association::Element);
        attach(output,attributes,ids,elementRows,item.id,"Topology Preserved",3,SimResults::Association::Element);
      }
      if (levelSet && item.point_count)
      {
        auto* attributes = grid->GetPointData();
        attach(output,attributes,attributes->GetArray("OriginalNodeId"),nodeRows,item.id,
          "Topology Level-Set Coefficient",1,SimResults::Association::Node);
      }
      collection->SetPartition(static_cast<unsigned int>(item.id),0,grid.get());
      datasets.push_back(std::move(item));
    }
  }
  if (identity.size() != elementRows.size() || identity.empty())
  {
    error = "The converted mesh does not cover the topology design domain by original element ID.";
    return false;
  }
  output.dataset = std::move(collection);
  return true;
}
