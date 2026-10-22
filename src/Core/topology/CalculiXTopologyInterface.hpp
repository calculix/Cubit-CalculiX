#ifndef CALCULIXTOPOLOGYINTERFACE_HPP
#define CALCULIXTOPOLOGYINTERFACE_HPP

#include <string>
#include <vector>
#include <set>

struct TopologyRunInfo;
struct TopologyOptions;
struct TopologyPreservedSetInfo;

class CalculiXTopologyInterface
{
public:
  bool run_topology(int topology_id, bool wait = false);
  bool wait_topology(int topology_id);
  bool kill_topology(int topology_id);
  bool get_topology_run_info(int topology_id, TopologyRunInfo& information);
  int run_topology_iteration(int topology_id, int iteration_id);// Returns the completed job ID, or -1 on failure.
  bool create_topology(int topology_id, const TopologyOptions& options);
  bool modify_topology(int topology_id, const TopologyOptions& options);
  bool delete_topology(int topology_id); // delete topology and all of its data
  bool get_topology_preserved_sets(int topology_id,std::vector<TopologyPreservedSetInfo>& sets,std::set<int>& direct_elements);
  bool reset_preserved_sets(int topology_id);
  bool add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);
  bool remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);
  bool get_topology_set_element_ids(int topology_id,int set_type,int set_id,std::vector<int>& element_ids);
  bool initialize_topology(int topology_id, const std::vector<int>& section_ids);
  std::vector<std::vector<int>> get_topology_element_preservation(int topology_id);
  bool set_topology_iteration_factors(int topology_id,int iteration_id,const std::vector<int>& element_ids,const std::vector<double>& stiffness_factors);
  bool check_topology_material_assignment(int topology_id, int iteration_id, int element_id);
  std::string get_topology_material_name(int topology_id, int element_id);
  bool get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name);
  std::vector<double> get_topology_parameters(int topology_id);
  bool get_topology_settings(int topology_id, TopologyOptions& options);
  int get_topology_iteration_job_id(int topology_id, int iteration_id); // Returns the associated job ID, or -1 on failure.
  int get_topology_iteration_adjoint_job_id(int topology_id, int iteration_id);
  std::vector<std::vector<std::string>> get_topology_iteration_data(int topology_id);
  bool select_topology_iteration(int topology_id, int iteration_id);
  std::string get_topology_run_state(int topology_id);
  std::vector<std::vector<double>> get_topology_iteration_element_data(int topology_id,int iteration_id = -1);
  bool is_topology_adjoint_export() const;
  bool check_topology_export(int topology_id, int iteration_id);
  std::vector<std::vector<std::string>> get_topology_tree_data();  // gets the data from core to build the tree
};

#endif // CALCULIXTOPOLOGYINTERFACE_HPP
