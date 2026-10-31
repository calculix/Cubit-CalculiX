#ifndef CORETOPOLOGYEVALUATION_HPP
#define CORETOPOLOGYEVALUATION_HPP

#include <map>

class CoreTopology;
class CoreMaterials;
class CoreJobs;

class CoreTopologyEvaluation
{
public:
  static bool evaluate_compliance(CoreTopology& topology, const CoreMaterials& materials, int topology_id, int iteration_id, const std::map<int, double>& energies);
  static bool evaluate_displacement(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals);
};

#endif
