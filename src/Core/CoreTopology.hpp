#ifndef CORETOPOLOGY_HPP
#define CORETOPOLOGY_HPP

#include "CoreTopologyStructs.hpp"

#include <map>
#include <string>
#include <vector>

class CalculiXCoreInterface;

class CoreTopology
{
public: 
  CoreTopology();
  ~CoreTopology();
  bool init(); // initialize
  bool update(); // check for changes of the constraints
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized

  bool create_topology(int topology_id, const TopologyOptions& options);
  bool modify_topology(int topology_id, const TopologyOptions& options);
  bool delete_topology(int topology_id);

  bool add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);
  bool remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);
  bool resolve_preserved_elements(int topology_id,const std::vector<int>& analysis_element_ids,const std::map<int, std::set<int>>& default_sets,const std::set<int>& default_elements);

  std::vector<std::vector<std::string>> get_tree_data() const;
  
  bool is_initialized = false;
  std::map<int, TopologyCase> topology_cases;
  CalculiXCoreInterface *ccx_iface = nullptr;

private:
  bool change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding);
};

#endif