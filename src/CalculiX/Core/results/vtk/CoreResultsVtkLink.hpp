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
   * Current-model coordinates are converted to the recorded source system
   * before comparison. Unknown source units use the INP/FRD fallback.
   */
  bool prepare(int job_id, const CoreResultsFrd& frd, const CoreResultsDat& dat, const std::string& input_path, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& output, const std::string& source_system = {}, const ConnectionRunMap* connections = nullptr) const;

  // External INP is authoritative when supplied. Never capture the live model.
  // A DAT-only source without an INP produces tables, not invented coordinates.
  bool prepare_external(const CoreResultsFrd& frd, const CoreResultsDat& dat,
    const std::string& input_path, const CoreResultsVtkOptions& options,
    CoreResultsVtkPlan& output, std::string& error) const;

  // Check the deck against the loaded FRD mesh before borrowing its unit tags.
  // On success, input_files contains the root deck and the includes read.
  static bool match_input_mesh(const CoreResultsFrd& frd, const std::string& input_path,
    std::vector<std::string>& input_files, std::string& error);

  // Pure snapshot comparison. Both snapshots must use unique sorted IDs and
  // CalculiX connectivity. Extra candidate nodes (e.g. unused reference nodes)
  // are permitted; analysis element IDs/connectivity must match exactly.
  static bool compatible(const CoreResultsVtkMesh& result_mesh, const CoreResultsVtkMesh& candidate, const CoreResultsVtkOptions& options, std::string& reason);
};

#endif // CORERESULTSVTKLINK_HPP
