#ifndef CORETOPOLOGYDISPLACEMENTRESULTS_HPP
#define CORETOPOLOGYDISPLACEMENTRESULTS_HPP

#include <array>
#include <map>

class CoreResults;
class CoreJobs;
class CoreTopology;
class MeshExportInterface;

class CoreTopologyDisplacementResults
{
public:
  static bool read_final(CoreResults& results, MeshExportInterface& mesh, int job_id, int nodeset_id, int component, std::map<int, std::array<double, 3>>& displacements);
  static bool measure_iteration(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, MeshExportInterface& mesh, int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement);
};

#endif
