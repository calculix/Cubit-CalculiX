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
  bool set_iteration_element_energies(int topology_id,int iteration_id,const std::map<int, double>& element_energies);


  /*
  Element stiffness:           K_e = s_e K_e_base
  Element internal energy:     U_e = ½ u_eᵀ K_e u_e
  Compliance:                  C = 2 Σ U_e
  Stiffness-factor derivative: dC/ds_e = -2 U_e / s_e
  */
  bool evaluate_compliance(int topology_id,int iteration_id, const std::map<int, double>& stiffness_factors);

  /*
  ρ = solid fraction, between 0 and 1
  s(ρ) = void_ratio + (1 − void_ratio) × ρ³
  */
  bool build_density_design(int topology_id,int iteration_id,const std::map<int, double>& densities,TopologyDensityDesign& design,std::map<int, double>& stiffness_factors);

  /*
  Total volume:       Σ V_e
  Solid volume:       Σ ρ_e V_e
  Preserved volume:   Σ V_e for preserved elements
  Volume fraction:    solid volume / total volume
  Minimum fraction:   preserved volume / total volume
  */
  bool evaluate_density_volume(int topology_id,int iteration_id,const std::map<int, double>& element_volumes);

  /*
  ρ_new = clamp(
    ρ × sqrt((-dC/dρ) / (λ × dVfraction/dρ)),
    ρ − move_limit,
    ρ + move_limit
    )
  */
  bool build_density_update(int topology_id, int source_iteration,std::map<int, double>& densities);

  bool check_density_convergence(int topology_id,int previous_iteration,int current_iteration,bool& converged);

  std::vector<std::vector<std::string>> get_tree_data() const;
  
  bool is_initialized = false;
  std::map<int, TopologyCase> topology_cases;
  CalculiXCoreInterface *ccx_iface = nullptr;

private:
  bool change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding);
};

#endif