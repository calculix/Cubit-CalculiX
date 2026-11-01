#ifndef CORETOPOLOGYQUERY_HPP
#define CORETOPOLOGYQUERY_HPP

#include <string>
#include <vector>

class CoreTopology;
class CoreJobs;
class CoreMaterials;

class CoreTopologyQuery
{
public:
  static int get_iteration_job_id(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id);
  static int get_iteration_adjoint_job_id(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id);
  static int get_active_iteration_job(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id);
  static std::vector<std::vector<std::string>> get_iteration_data(const CoreTopology& topology, CoreJobs& jobs, int topology_id);
  static std::vector<std::vector<double>> get_iteration_element_data(const CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id);
  static bool get_job_context(const CoreTopology& topology, int job_id, int& topology_id, int& iteration_id);
  static int get_adjoint_job_source(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id);
  static bool validate_selected_iterations(const CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs);
};

#endif
