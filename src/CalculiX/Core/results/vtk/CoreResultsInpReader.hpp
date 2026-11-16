#ifndef CORERESULTSINPREADER_HPP
#define CORERESULTSINPREADER_HPP

#include "CoreResultsVtkStructs.hpp"
#include <string>
#include <vector>

class CoreResultsInpReader
{
public:
  /** Read only mesh, NSET/ELSET and element-based SURFACE definitions.
   * INCLUDE is relative to its containing file; recursion/cycles are rejected.
   * Sets may reference sets declared later and GENERATE requires a positive stride.
   * CUBIT_SIM_SURFACE_HELPER_ELSET comments mark the next ELSET keyword in the
   * same file as internal surface support. All sets remain resolved/validated;
   * ordinary ELSET or ELEMENT/ELSET declarations preserve standalone groups.
   * Unmarked legacy sets are never classified by name or surface usage.
   * Unsupported mesh-changing keywords fail instead of supplying a partial mesh.
   * Outputs are replaced only on success. No solver/model state is modified.
   */
  bool read(const std::string& filename, CoreResultsVtkMesh& mesh, std::vector<std::string>& input_files, std::string& error) const;
};

#endif // CORERESULTSINPREADER_HPP
