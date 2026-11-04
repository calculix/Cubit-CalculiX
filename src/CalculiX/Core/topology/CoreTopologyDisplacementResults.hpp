#ifndef CORETOPOLOGYDISPLACEMENTRESULTS_HPP
#define CORETOPOLOGYDISPLACEMENTRESULTS_HPP

#include <array>
#include <map>

class CoreResults;
class CoreJobs;
class CoreTopology;
class MeshExportInterface;

/** @brief Measure displacement objectives from loaded final-frame DAT nodeset results. */
class CoreTopologyDisplacementResults
{
public:
  /**
   * @brief Collect final DAT displacement vectors for a monitored nodeset.
   * @param results Borrowed store containing loaded DAT results.
   * @param mesh Borrowed interface resolving the nodeset and required nodes.
   * @param job_id Job whose DAT association identifies the result data.
   * @param nodeset_id Positive monitored Cubit nodeset ID.
   * @param component Requested measurement: 0=magnitude, 1=X, 2=Y, 3=Z; XYZ columns are read.
   * @param[out] displacements Monitored node ID -> XYZ vector for maximum selection.
   * @return True for complete required data; false with cleared output otherwise.
   * @note This is the DAT objective-measurement path, distinct from the FRD reader.
   * Missing data at the final parser step/increment is an error. No earlier-frame
   * fallback, solver execution or file loading occurs here.
   */
  static bool read_final(CoreResults& results, MeshExportInterface& mesh, int job_id, int nodeset_id, int component, std::map<int, std::array<double, 3>>& displacements);
  /**
   * @brief Measure the configured maximum displacement in a completed physical iteration.
   * @param results Borrowed loaded-result store.
   * @param jobs Borrowed job store; the physical job must have finished status.
   * @param topology Borrowed case store supplying nodeset, component and job association.
   * @param mesh Borrowed interface resolving the monitored nodeset.
   * @param topology_id Existing case with a displacement nodeset configured.
   * @param iteration_id Positive physical iteration to measure.
   * @param[out] value Maximum absolute component or magnitude; initialized to zero.
   * @param[out] node_id Controlling node ID; initialized to -1.
   * @param[out] critical_displacement Optional selected XYZ vector; initialized to zero.
   * @return True after final-DAT reading and maximum selection; false otherwise.
   * @note Does not store an evaluation. Exact ties select the lowest node ID;
   * zero is a valid measured objective and is not an availability marker.
   */
  static bool measure_iteration(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, MeshExportInterface& mesh, int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement);
};

#endif
