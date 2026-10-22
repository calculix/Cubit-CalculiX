#ifndef CORETOPOLOGY_HPP
#define CORETOPOLOGY_HPP
#include <set>

#include "Core/topology/CoreTopologyStructs.hpp"

#include <map>
#include <string>
#include <vector>
#include <array>
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

  /**
   * @brief Create a case by applying supplied options to core-owned defaults.
   * @param topology_id Positive, unused topology case ID.
   * @param options Only entries with their has_* flag set override the defaults.
   * @return True when the validated case is stored; false on an invalid request.
   * @note Creation does not start optimization or establish run-time model validity.
   */
  bool create_topology(int topology_id, const TopologyOptions& options);

  /**
   * @brief Modify a case while preserving selections and stored iterations.
   * @param topology_id Existing topology case ID that is not running.
   * @param options Only entries with their has_* flag set replace current values.
   * @return True when the validated copy replaces the case; false otherwise.
   * @note A rejected modification leaves the stored case unchanged. Earlier
   * evaluations are not recomputed; starting again resets that case's run data.
   */
  bool modify_topology(int topology_id, const TopologyOptions& options);
  bool delete_topology(int topology_id);

  /**
   * @brief Clear inclusion/exclusion overrides for subsequent runs.
   * @param topology_id Existing case ID that is not running.
   * @return True when both override maps are cleared; false on an invalid request.
   * @note Automatic preservation is retained. Existing element flags and
   * iteration records are unchanged until subsequent run preparation/reset.
   */
  bool reset_preserved_sets(int topology_id);

  /**
   * @brief Include sets and cancel their exclusions for subsequent runs.
   * @param topology_id Existing case ID that is not running.
   * @param set_type Preservation kind: 1=block, 2=nodeset, 3=sideset.
   * @param ids Existing positive set IDs of the specified kind.
   * @return True when the validated list is applied; false otherwise.
   * @note Validates the complete list before changing rules. Repeated IDs
   * are idempotent; existing resolved flags and iteration records are retained.
   */
  bool add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);

  /**
   * @brief Exclude sets and cancel their explicit inclusions for subsequent runs.
   * @param topology_id Existing case ID that is not running.
   * @param set_type Preservation kind: 1=block, 2=nodeset, 3=sideset.
   * @param ids Existing positive set IDs of the specified kind.
   * @return True when the validated list is applied; false otherwise.
   * @note Exclusion removes a set rule, not its elements from the final union.
   * Overlapping effective sets and direct selections can still preserve them.
   * Existing resolved flags and iteration records are retained.
   */
  bool remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);

  /**
   * @brief Resolve case rules into a flag for every participating element.
   * @param topology_id Existing case whose analysis/material collection is prepared.
   * @param analysis_element_ids Nonempty, unique positive global element IDs.
   * @param default_sets Automatic sets keyed by kind: 1=block, 2=nodeset, 3=sideset.
   * @param default_elements Direct automatic selections, independent of set exclusions.
   * @return True after committing all flags; false without replacing the old flags.
   * @note Effective sets are (automatic union included) minus excluded. Their
   * resolved elements are unioned with direct selections inside the analysis
   * domain. Positive direct IDs outside the domain are ignored; invalid IDs
   * or failed set queries reject resolution. All flags start false locally.
   */
  bool resolve_preserved_elements(int topology_id,const std::vector<int>& analysis_element_ids,const std::map<int, std::set<int>>& default_sets,const std::set<int>& default_elements);
  bool set_iteration_element_energies(int topology_id,int iteration_id,const std::map<int, double>& element_energies);


  /**
   * @brief Commit compliance and raw derivatives from physical element energies.
   * @param topology_id Existing prepared density case.
   * @param iteration_id Unevaluated iteration with stored element energies.
   * @param stiffness_factors Complete element factors consistent with its design.
   * @return True after storing the evaluation; false on invalid or incomplete data.
   * @note For fixed loads and homogeneous prescribed displacements, C = 2 sum(Ue)
   * and dC/ds = -2 Ue/s. Density derivatives multiply by (1-v) p rho^(p-1).
   * Preserved derivatives are zero. No sensitivity filtering is performed here.
   */
  bool evaluate_compliance(int topology_id,int iteration_id, const std::map<int, double>& stiffness_factors);

  /**
   * @brief Commit the displacement objective and raw adjoint derivatives.
   * @param topology_id Prepared displacement case with its nodeset/component.
   * @param iteration_id Unevaluated iteration with a stored density design.
   * @param value Finite, nonnegative maximum displacement objective.
   * @param node_id Controlling node selected from the displacement nodeset.
   * @param element_integrals Complete integrals of physical S contracted with adjoint ME.
   * @param stiffness_factors Applied solver factors for the stored iteration.
   * @param grouped_materials Applied factors may approximate continuous SIMP factors.
   * @return True after storing the evaluation; false on inconsistent data.
   * @note dJ/ds = -I/s; multiply by (1-v) p rho^(p-1) for density derivatives.
   * With grouped materials the smooth SIMP density derivative is an approximation.
   * The bilinear contraction has no factor of one half. Preserved derivatives
   * are zero. A zero objective requires zero integrals and no adjoint job.
   */
  bool evaluate_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals, const std::map<int, double>& stiffness_factors, bool grouped_materials = false);

  /**
   * @brief Build a validated SIMP design and its element stiffness factors.
   * @param topology_id Prepared density case defining the element domain.
   * @param iteration_id Positive iteration ID for the design.
   * @param densities Complete element densities within the permitted bounds.
   * @param design Output design including interpolation settings.
   * @param stiffness_factors Output factors s = v + (1-v) rho^p.
   * @return True on success; false for invalid coverage, values, or preservation.
   * @note Preserved densities are forced to one. The caller stores the design.
   */
  bool build_density_design(int topology_id,int iteration_id,const std::map<int, double>& densities,TopologyDensityDesign& design,std::map<int, double>& stiffness_factors);

  /**
   * @brief Store volume diagnostics and free-variable volume derivatives.
   * @param topology_id Prepared density case.
   * @param iteration_id Stored density design to measure.
   * @param element_volumes Complete, finite, positive physical element volumes.
   * @return True when diagnostics are stored; false on invalid data.
   * @note Fraction = sum(rho V)/sum(V). The minimum fraction field contains
   * preserved volume/sum(V), excluding the free elements' minimum densities.
   * Free derivatives are V/sum(V); preserved derivatives are zero.
   */
  bool evaluate_density_volume(int topology_id,int iteration_id,const std::map<int, double>& element_volumes);

  /**
   * @brief Propose the next densities from a completed source evaluation.
   * @param topology_id Density case defining objective, bounds, and volume target.
   * @param source_iteration Stored design with filtered sensitivities and volume data.
   * @param densities Output proposed element densities.
   * @return True for a valid proposal; false if required data or calculations fail.
   * @note Compliance uses OC with optional adaptive moves; displacement uses a
   * signed projected-gradient update. Both preserve fixed elements and stage
   * the volume target to respect move bounds. Neither performs a new solve.
   */
  bool build_density_update(int topology_id, int source_iteration,std::map<int, double>& densities);

  /**
   * @brief Store max(abs(rho_OC-rho)) using the configured maximum move limit.
   * @param topology_id Compliance density case.
   * @param iteration_id Evaluated design with filtered sensitivities and volume data.
   * @return True after storing an available residual; false on invalid data.
   * @note Clears residual availability before calculation. The diagnostic ignores
   * adaptive move reduction and does not store or solve the proposed design.
   */
  bool evaluate_compliance_oc_residual(int topology_id, int iteration_id);

  /**
   * @brief Attach the source update's effective move and correlation to a design.
   * @param topology_id Density case supplying update settings/history.
   * @param source_iteration Source iteration used to select the move limit.
   * @param design Next design receiving update diagnostics.
   * @return True when diagnostics can be populated; false on invalid data.
   */
  bool set_density_update_diagnostics(int topology_id, int source_iteration, TopologyDensityDesign& design) const;

  /**
   * @brief Store convergence diagnostics for two evaluated density designs.
   * @param topology_id Density case defining tolerances and volume target.
   * @param previous_iteration Earlier positive iteration ID.
   * @param current_iteration Later evaluated iteration ID.
   * @param converged Output initialized false; true only when every test passes.
   * @return Operation success, independently of the converged output.
   * @note Tests maximum density change, relative objective change using the larger
   * objective as denominator (zero when both are zero), and volume <= target+1e-6.
   * Compliance also needs an available OC residual within the density tolerance.
   */
  bool check_density_convergence(int topology_id,int previous_iteration,int current_iteration,bool& converged);

  /**
   * @brief Store volume-aware filtered sensitivities without changing raw derivatives.
   * @param topology_id Prepared density case defining preservation and filter radius.
   * @param iteration_id Evaluated stored density design.
   * @param element_volumes Complete finite, positive physical volumes.
   * @param element_centers Complete finite global element centers.
   * @return True after committing filtered values; false without replacing them.
   * @note Free neighbors within r use a_ij=(1-distance/r) Vj/Vmax and
   * filtered_gi=(Vi/rhoi) sum(a_ij rhoj gj/Vj)/sum(a_ij).
   * Radius zero selects 1.5 cbrt(total_volume/element_count). Preserved elements
   * are excluded and get zero. Success invalidates the ordinary OC residual.
   */
  bool filter_density_sensitivities(int topology_id,int iteration_id,const std::map<int, double>& element_volumes,const std::map<int, std::array<double, 3>>& element_centers);

  std::vector<std::vector<std::string>> get_tree_data() const;
  
  bool is_initialized = false;
  std::map<int, TopologyCase> topology_cases;

private:
  bool select_compliance_move_limit(const TopologyCase& configuration, int source_iteration, double& move_limit, bool& correlation_available, double& correlation) const;
  bool build_compliance_oc_proposal(const TopologyCase& configuration, const TopologyDensityDesign& design, const TopologyIterationEvaluation& evaluation, double move_limit, std::map<int, double>& densities) const;
  bool build_displacement_density_update(const TopologyCase& configuration, const TopologyDensityDesign& design, const TopologyIterationEvaluation& evaluation, std::map<int, double>& densities) const;
  bool change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding);
};

#endif
