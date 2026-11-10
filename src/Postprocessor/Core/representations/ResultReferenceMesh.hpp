#ifndef POSTPROCESSOR_RESULTREFERENCEMESH_HPP
#define POSTPROCESSOR_RESULTREFERENCEMESH_HPP
#include "Framework/Results/VtkResultsData.hpp"
#include <memory>
#include <string>
#include <vector>
class vtkDataSet;
namespace Postprocessor
{
  // Same-source/frame lookup using integral OriginalElementId, never names or
  // rendering indices. Inputs remain immutable; outputs own original FE cells.
  // Repeated IDs are deduplicated only when type, ordered node identity (when
  // present) and coordinates agree exactly within this immutable snapshot.
  // Missing/ambiguous parents return success with a warning and are omitted.
  // Extraction failure returns false; output remains unchanged on failure.
  bool integration_point_reference_meshes(const SimResults::Increment& frame,
    vtkDataSet* integrationPoints, std::vector<std::shared_ptr<vtkDataSet>>& output,
    std::string& warning);
}
#endif
