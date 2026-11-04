#ifndef CORETOPOLOGYRESULTS_HPP
#define CORETOPOLOGYRESULTS_HPP

#include <array>
#include <map>

class CoreResults;
class CoreJobs;
class CoreTopology;
class CoreMaterials;
class CoreBlocks;

/** @brief Read FRD displacements, DAT energies and select monitored maxima. */
class CoreTopologyResults
{
public:
  /**
   * @brief Read global nodal displacement vectors from the final FRD frame.
   * @param results Borrowed store containing loaded FRD data.
   * @param job_id Job whose FRD association identifies the result data.
   * @param[out] displacements Node ID -> finite XYZ displacement vector.
   * @return True for nonempty valid results; false with empty output otherwise.
   * @note Clears output on entry and commits on success. No earlier-frame fallback
   * occurs. This reads FRD, not the DAT nodeset data used to measure the objective;
   * finished-job and topology-node coverage checks belong to read_job_displacements().
   */
  static bool read_final_displacements(CoreResults& results, int job_id, std::map<int, std::array<double, 3>>& displacements);
  /**
   * @brief Read final FRD displacements and require every prepared topology node.
   * @param results Borrowed loaded-result store.
   * @param jobs Borrowed job store; the job must have finished status.
   * @param topology Borrowed case store supplying the job context.
   * @param materials Borrowed prepared-element assignments for that case.
   * @param blocks Required block/type mapping pointer; null is rejected.
   * @param job_id Finished physical or adjoint topology job.
   * @param[out] displacements Final FRD vectors, including all required topology nodes.
   * @return True after reading and coverage validation; false with cleared output otherwise.
   * @note Includes expanded-connectivity midside nodes in the required coverage.
   * The returned FRD map may also contain nodes outside the topology domain.
   */
  static bool read_job_displacements(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, CoreBlocks* blocks, int job_id, std::map<int, std::array<double, 3>>& displacements);
  /**
   * @brief Read whole-element internal energies from the final DAT frame.
   * @param results Borrowed store containing loaded DAT data.
   * @param job_id Job whose DAT association identifies the result data.
   * @param element_materials Required global element IDs in the keys; material IDs are not used.
   * @param[out] element_energies Global element ID -> internal strain energy.
   * @return True for complete valid coverage; false with empty output otherwise.
   * @note Reads ELSE/internalenergy, not energy density or a nodal field. Output
   * is cleared on entry and committed on success. Does not fall back to an older
   * frame or verify finished-job status; read_iteration_energies() supplies context checks.
   */
  static bool read_final_element_energies(CoreResults& results, int job_id, const std::map<int, int>& element_materials, std::map<int, double>& element_energies);
  /**
   * @brief Select a node attaining the largest absolute component or vector norm.
   * @param measured Ordered map of node IDs to displacement vectors.
   * @param component 0=magnitude, 1=X, 2=Y, 3=Z.
   * @param[out] value Maximum value; initialized to zero.
   * @param[out] node_id Controlling node ID; initialized to -1.
   * @param[out] critical_displacement Optional selected XYZ vector; initialized to zero.
   * @return True for a nonempty map and supported component; false otherwise.
   * @pre Input vectors and node IDs have already been validated by the reader.
   * @note Exact ties retain the lowest node ID. This method does not independently
   * validate vector finiteness and does not establish differentiability at a tie or zero.
   */
  static bool select_maximum_displacement(const std::map<int, std::array<double, 3>>& measured, int component, double& value, int& node_id, std::array<double, 3>* critical_displacement);
  /**
   * @brief Validate a completed physical iteration and read its final DAT energies.
   * @param results Borrowed loaded-result store.
   * @param jobs Borrowed job store requiring finished status.
   * @param topology Borrowed case store supplying the physical job association.
   * @param materials Borrowed store supplying required element assignments.
   * @param topology_id Prepared case ID.
   * @param iteration_id Positive iteration whose physical job is finished.
   * @param[out] element_energies Global element ID -> internal strain energy.
   * @return True for complete valid data; false with cleared output otherwise.
   * @note Does not load files or commit an objective evaluation. Frame and coverage
   * requirements follow read_final_element_energies().
   */
  static bool read_iteration_energies(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, int topology_id, int iteration_id, std::map<int, double>& element_energies);
};

#endif
