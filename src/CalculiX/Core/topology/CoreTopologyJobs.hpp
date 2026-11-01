#ifndef CORETOPOLOGYJOBS_HPP
#define CORETOPOLOGYJOBS_HPP

#include <functional>
#include <string>

class CoreSections;
class CoreTopologyOptimization;
class CoreMaterials;
class CoreTopology;
class CoreJobs;

class CoreTopologyJobs
{
public:
  static bool register_adjoint_job(CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id, int& job_id);
  static int create_iteration_job(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id, const std::string& job_name, const std::function<bool()>& validate_model);
  static bool reset_run(CoreTopology& topology, CoreJobs& jobs, CoreMaterials* materials, CoreSections* sections, CoreTopologyOptimization* optimization, int topology_id);
  static bool delete_topology(CoreTopology& topology, CoreJobs& jobs, CoreMaterials* materials, CoreSections* sections, CoreTopologyOptimization* optimization, int topology_id);
  static void invalidate_saved_processes(const CoreTopology& topology, CoreJobs& jobs);
};

#endif
