#ifndef POSTPROCESSOR_RESULTMESH_HPP
#define POSTPROCESSOR_RESULTMESH_HPP

#include "Framework/Results/VtkResultsData.hpp"
#include <array>
#include <optional>
#include <vector>

class vtkDataSet;

namespace Postprocessor
{
  // Borrowed mesh pointers: the caller must retain the owning result handle.
  // Empty datasets and non-mesh objects are skipped; buffers are never modified.
  std::vector<vtkDataSet*> result_meshes(vtkDataObject* object);
  std::optional<std::size_t> first_mesh_increment(const SimResults::VtkResultsData& data);
  bool mesh_bounds(vtkDataSet* mesh, std::array<double, 6>& bounds);
}

#endif
