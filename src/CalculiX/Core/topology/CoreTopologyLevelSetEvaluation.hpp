#ifndef CORETOPOLOGYLEVELSETEVALUATION_HPP
#define CORETOPOLOGYLEVELSETEVALUATION_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <map>
#include <vector>

struct TopologyLevelSetDesign;

class CoreTopologyLevelSetEvaluation
{
public:
  /**
   * @brief Query the current Cubit mesh and evaluate all element fractions/volumes.
   * @return False without modifying design on failure; inputs must be the complete domain.
   * @note Volumes and preservation must have identical element IDs. The field
   * must contain exactly the domain nodes; fixed_nodes must equal the union of
   * expanded connectivity of preserved elements, with coefficients exactly +1.
   * Initial designs require a resolved free interface. Cubit volumes, not
   * geometric normalization volumes, define the constraint. No model store changes.
   */
  static bool build_geometry(TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes);

  /**
   * @brief Build requested s=v+(1-v)*alpha factors, forcing preserved factors to one.
   * @return False with empty output on invalid/incomplete input.
   * @note No material store is modified; the controller will use the existing
   * topology factor interface. Output must not alias the design's fraction map.
   */
  static bool build_factors(const TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, std::map<int, double>& factors);

  /**
   * @brief Assemble compliance and nodal derivatives at a solved applied design.
   * @return False with an unavailable/empty output on invalid/incomplete input.
   * @note Requires fixed loads, homogeneous prescribed displacement and symmetric
   * linear elasticity, as for density compliance. C=2*sum(Ue), dC/ds=-2*Ue/s.
   * The chain rule uses the continuous requested interpolation; grouped/rounded
   * material solves therefore supply surrogate derivatives, not derivatives of
   * bin assignment or input text rounding. Fixed-node derivatives are zero.
   * The caller verifies physical job completion and unchanged mesh before use.
   * No stored evaluation, material or job is changed by this helper.
   */
  static bool evaluate_compliance(const TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, const std::map<int, double>& element_energies, const std::map<int, double>& applied_factors, TopologyLevelSetEvaluation& evaluation);

  /**
   * @brief Accumulate one element into an unfinished nodal evaluation.
   * @return False with a Cubit error and cleared evaluation on invalid data.
   * @note The caller initializes iteration_id and zero derivative maps for the
   * complete design node domain. Subsequent calls add to those same maps.
   * Uses dC/ds=-2*U/s and ds/dphi=(1-void_ratio)*d(alpha)/dphi; fixed
   * coefficients are not varied. Input geometry is already evaluated in Cubit
   * node order. This math-only helper does not verify live mesh membership,
   * resolve preservation, or set factor_approximation. No model store changes.
   */
  static bool accumulate_compliance(const TopologyLevelSetDesign& design, const std::vector<int>& nodes, const TopologyLevelSetElementGeometry& geometry, double element_volume, double element_energy, double applied_factor, TopologyLevelSetEvaluation& evaluation);

  /**
   * @brief Normalize assembled volume derivatives and mark evaluation available.
   * @return False with a Cubit error and cleared evaluation on invalid data.
   * @note Total participating volume is constant during differentiation. Call
   * once after all elements and live-domain validation; the helper cannot
   * establish coverage of a Cubit mesh. No stored record is modified.
   */
  static bool finalize_compliance(TopologyLevelSetEvaluation& evaluation);
};

#endif
