#ifndef POSTPROCESSOR_RESULTMESH_HPP
#define POSTPROCESSOR_RESULTMESH_HPP

#include "Framework/Results/VtkResultsData.hpp"
#include <array>
#include <optional>
#include <vector>
#include <utility>

class vtkDataSet;

namespace Postprocessor
{
  // Borrowed mesh pointers: the caller must retain the owning result handle.
  // Empty datasets and non-mesh objects are skipped; buffers are never modified.
  std::vector<vtkDataSet*> result_meshes(vtkDataObject* object);
  std::vector<std::pair<std::size_t, vtkDataSet*>> result_mesh_parts(vtkDataObject* object);
  std::optional<std::size_t> first_mesh_increment(const SimResults::VtkResultsData& data);
  bool mesh_bounds(vtkDataSet* mesh, std::array<double, 6>& bounds);
  // Union of actual mesh IDs across frames; metadata names are copied by ID,
  // never by vector position. Missing metadata does not hide independent grids.
  std::vector<SimResults::Dataset> result_dataset_catalogue(const SimResults::VtkResultsData& data);
}

#endif
