#ifndef COREMATERIALSTOPOLOGYGROUPING_HPP
#define COREMATERIALSTOPOLOGYGROUPING_HPP

#include <map>

/**
 * @brief Quantize stiffness factors using fixed levels and hysteresis.
 * @note Budgets depend on base-material element counts, never current distinct
 * factors. Levels span [minimum_factor,1] with uniform absolute spacing. A base
 * whose budget covers its elements remains exact. Nearest-level assignment is
 * retained within a band of 0.1 level spacings around the previous bin boundary.
 * Physical endpoints are exact. Previous factors must match the same palette
 * and domain. Fixed cap, bounds and assignments make the palette reproducible;
 * historical export must instead reconstruct its stored applied factors exactly.
 * Hysteresis reduces switching but does not certify convergence or accuracy.
 * @return False with a Cubit error and empty output on invalid data. Aliased
 * outputs are rejected without modifying their inputs. No model store is changed.
 */
class CoreMaterialsTopologyGrouping
{
public:
  static bool apply(const std::map<int, int>& element_base_material, const std::map<int, double>& requested, const std::map<int, double>* previous, int material_cap, double minimum_factor, std::map<int, double>& applied);
};

#endif
