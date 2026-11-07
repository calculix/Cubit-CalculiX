#include "ResultMesh.hpp"
#include <vtkDataSet.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <algorithm>
#include <cmath>

namespace Postprocessor
{
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
