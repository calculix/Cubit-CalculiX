#include "ResultMesh.hpp"
#include <vtkDataSet.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <algorithm>
#include <cmath>
#include <map>

namespace Postprocessor
{
  std::vector<SimResults::Dataset> result_dataset_catalogue(const SimResults::VtkResultsData& data)
  {
    std::map<std::size_t,SimResults::Dataset> found;
    for (const auto& frame : data.increments)
    {
      for (const auto& part : result_mesh_parts(frame.dataset.get()))
      {
        auto& entry = found[part.first];
        entry.id = part.first;
        entry.point_count = std::max(entry.point_count,static_cast<std::size_t>(part.second->GetNumberOfPoints()));
        entry.cell_count = std::max(entry.cell_count,static_cast<std::size_t>(part.second->GetNumberOfCells()));
      }
    }
    std::vector<SimResults::Dataset> result;
    for (auto& entry : found)
    {
      const auto metadata = std::find_if(data.datasets.begin(),data.datasets.end(),[&](const SimResults::Dataset& value)
      {
        return value.id == entry.first;
      });
      if (metadata != data.datasets.end())
      {
        entry.second.name = metadata->name;
        entry.second.kind = metadata->kind;
        entry.second.model_id = metadata->model_id;
        entry.second.element_type = metadata->element_type;
      }
      result.push_back(std::move(entry.second));
    }
    return result;
  }

  bool mesh_bounds(vtkDataSet* mesh, std::array<double, 6>& bounds)
  {
    if (mesh == nullptr || mesh->GetNumberOfPoints() == 0 || mesh->GetNumberOfCells() == 0)
    {
      return false;
    }
    mesh->GetBounds(bounds.data());
    return std::all_of(bounds.begin(), bounds.end(), [](double value)
    {
      return std::isfinite(value);
    }) && bounds[0] <= bounds[1] && bounds[2] <= bounds[3] && bounds[4] <= bounds[5];
  }

  std::vector<vtkDataSet*> result_meshes(vtkDataObject* object)
  {
    std::vector<vtkDataSet*> meshes;
    const auto append = [&meshes](vtkDataSet* mesh)
    {
      std::array<double, 6> bounds;
      if (mesh_bounds(mesh, bounds))
      {
        meshes.push_back(mesh);
      }
    };
    if (auto* mesh = vtkDataSet::SafeDownCast(object))
    {
      append(mesh);
    }
    else if (auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(object))
    {
      for (unsigned int item = 0; item < collection->GetNumberOfPartitionedDataSets(); ++item)
      {
        auto* partitions = collection->GetPartitionedDataSet(item);
        if (partitions == nullptr)
        {
          continue;
        }
        for (unsigned int part = 0; part < partitions->GetNumberOfPartitions(); ++part)
        {
          append(partitions->GetPartition(part));
        }
      }
    }
    else if (auto* partitions = vtkPartitionedDataSet::SafeDownCast(object))
    {
      for (unsigned int part = 0; part < partitions->GetNumberOfPartitions(); ++part)
      {
        append(partitions->GetPartition(part));
      }
    }
    return meshes;
  }

  std::vector<std::pair<std::size_t, vtkDataSet*>> result_mesh_parts(vtkDataObject* object)
  {
    std::vector<std::pair<std::size_t, vtkDataSet*>> parts;
    if (auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(object))
    {
      for (unsigned int i = 0; i < collection->GetNumberOfPartitionedDataSets(); ++i)
      {
        for (auto* mesh : result_meshes(collection->GetPartitionedDataSet(i)))
        {
          parts.emplace_back(i, mesh);
        }
      }
    }
    else
    {
      for (auto* mesh : result_meshes(object))
      {
        parts.emplace_back(0, mesh);
      }
    }
    return parts;
  }

  std::optional<std::size_t> first_mesh_increment(const SimResults::VtkResultsData& data)
  {
    for (std::size_t index = 0; index < data.increments.size(); ++index)
    {
      if (!result_meshes(data.increments[index].dataset.get()).empty())
      {
        return index;
      }
    }
    return std::nullopt;
  }
}
