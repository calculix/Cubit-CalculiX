#ifndef CORETOPOLOGYTENSORRESULTS_HPP
#define CORETOPOLOGYTENSORRESULTS_HPP

#include <array>
#include <map>
#include <string>

class CoreResults;
class CoreJobs;
class CoreTopology;
class CoreMaterials;

/** @brief Read integration-point tensors without owning or reloading result stores. */
class CoreTopologyTensorResults
{
public:
  /**
   * @brief Read required element tensors from the parser's final DAT frame.
   * @param results Borrowed store containing loaded DAT results for job_id.
   * @param job_id Job whose DAT data is read; this low-level reader does not check job status.
   * @param key S for stresses or ME for mechanical strains; no other key is supported.
   * @param required_elements Required global element IDs in the keys; material IDs are not used.
   * @param[out] tensors Global element ID -> one-based integration-point ID -> six components.
   * @return True with results for every required element; false with empty output otherwise.
   * @note Clears output on entry and commits on success. Expects global-coordinate
   * xx, yy, zz, xy, xz, yz components; it does not transform coordinates. ME uses
   * tensor shear. Identical duplicate records are accepted; conflicting ones fail.
   * No earlier-frame fallback occurs. The objective evaluator, not this reader,
   * checks coverage against the complete expected quadrature-point set.
   */
  static bool read_final(CoreResults& results, int job_id, const std::string& key, const std::map<int, int>& required_elements, std::map<int, std::map<int, std::array<double, 6>>>& tensors);
  /**
   * @brief Validate a completed topology job, then read its final DAT tensors.
   * @param results Borrowed loaded-result store.
   * @param jobs Borrowed job store; job_id must have finished status.
   * @param topology Borrowed case store supplying physical/adjoint job associations.
   * @param materials Borrowed prepared-element assignments for the associated case.
   * @param job_id Finished physical or adjoint topology job.
   * @param key S or ME, with the component conventions of read_final().
   * @param[out] tensors Global element ID -> one-based point ID -> six components.
   * @return True after context validation and reading; false with cleared output otherwise.
   * @note Does not run a solver or load files. read_final() supplies frame, duplicate
   * and coordinate-basis expectations; expected quadrature coverage is checked later.
   */
  static bool read_job(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors);
};

#endif
