#ifndef CORETOPOLOGY_HPP
#define CORETOPOLOGY_HPP

#include "CoreTopologyStructs.hpp"

#include <map>
#include <string>
#include <vector>
#include <array>

class CalculiXCoreInterface;
class HDF5Tool;

class CoreTopology
{
public: 
  CoreTopology();
  ~CoreTopology();
  bool init(); // initialize
  bool update(); // check for changes of the constraints
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool save_configuration(HDF5Tool& file) const; // saving for .cub5
  bool read_configuration(HDF5Tool& file); // loading for .cub5

  bool create_topology(int topology_id, const TopologyOptions& options);
  bool modify_topology(int topology_id, const TopologyOptions& options);
  bool delete_topology(int topology_id);

  bool reset_preserved_sets(int topology_id);
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
   * I_e = integral(sigma_physical : epsilon_adjoint) dV
   * dJ/ds_e = -I_e / s_e
   * dJ/drho_e = (dJ/ds_e) (1 - void_ratio) p rho_e^(p - 1)
   */
  bool evaluate_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals, const std::map<int, double>& stiffness_factors);

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
  bool evaluate_compliance_oc_residual(int topology_id, int iteration_id);
  bool set_density_update_diagnostics(int topology_id, int source_iteration, TopologyDensityDesign& design) const;
  bool check_density_convergence(int topology_id,int previous_iteration,int current_iteration,bool& converged);
  bool filter_density_sensitivities(int topology_id,int iteration_id,const std::map<int, double>& element_volumes,const std::map<int, std::array<double, 3>>& element_centers);

  std::vector<std::vector<std::string>> get_tree_data() const;
  
  bool is_initialized = false;
  std::map<int, TopologyCase> topology_cases;
  CalculiXCoreInterface *ccx_iface = nullptr;

private:
  bool select_compliance_move_limit(const TopologyCase& configuration, int source_iteration, double& move_limit, bool& correlation_available, double& correlation) const;
  bool build_compliance_oc_proposal(const TopologyCase& configuration, const TopologyDensityDesign& design, const TopologyIterationEvaluation& evaluation, double move_limit, std::map<int, double>& densities) const;
  bool build_displacement_density_update(const TopologyCase& configuration, const TopologyDensityDesign& design, const TopologyIterationEvaluation& evaluation, std::map<int, double>& densities) const;
  bool change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding);
};

#endif
