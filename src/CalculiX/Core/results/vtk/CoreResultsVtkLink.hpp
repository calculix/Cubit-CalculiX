#ifndef CORERESULTSVTKLINK_HPP
#define CORERESULTSVTKLINK_HPP

#include "CoreResultsVtkStructs.hpp"
#include <string>

class CoreResultsFrd;
class CoreResultsDat;

class CoreResultsVtkLink
{
public:
  /** Host-thread operation. Check current model first, then matching job INP,
   * otherwise FRD-only. All captured connectivity is normalized to CalculiX.
   * The caller must keep model/readers stable for this entire call. Result
   * indices remain valid only until those readers are changed. No datasets are
   * allocated here; a successful plan does not mean conversion is complete.
   * Failure leaves output unchanged and reports a Cubit error.
   */
  bool prepare(int job_id, const CoreResultsFrd& frd, const CoreResultsDat& dat, const std::string& input_path, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& output) const;

  // Pure snapshot comparison. Both snapshots must use unique sorted IDs and
  // CalculiX connectivity. Extra candidate nodes (e.g. unused reference nodes)
  // are permitted; analysis element IDs/connectivity must match exactly.
  static bool compatible(const CoreResultsVtkMesh& result_mesh, const CoreResultsVtkMesh& candidate, const CoreResultsVtkOptions& options, std::string& reason);
};

#endif // CORERESULTSVTKLINK_HPP
