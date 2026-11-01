#ifndef CORETOPOLOGYRESULTS_HPP
#define CORETOPOLOGYRESULTS_HPP

#include <array>
#include <map>

class CoreResults;
class CoreJobs;
class CoreTopology;
class CoreMaterials;
class CoreBlocks;

class CoreTopologyResults
{
public:
  static bool read_final_displacements(CoreResults& results, int job_id, std::map<int, std::array<double, 3>>& displacements);
  static bool read_job_displacements(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, CoreBlocks* blocks, int job_id, std::map<int, std::array<double, 3>>& displacements);
  static bool read_final_element_energies(CoreResults& results, int job_id, const std::map<int, int>& element_materials, std::map<int, double>& element_energies);
  static bool select_maximum_displacement(const std::map<int, std::array<double, 3>>& measured, int component, double& value, int& node_id, std::array<double, 3>* critical_displacement);
  static bool read_iteration_energies(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, int topology_id, int iteration_id, std::map<int, double>& element_energies);
};

#endif
