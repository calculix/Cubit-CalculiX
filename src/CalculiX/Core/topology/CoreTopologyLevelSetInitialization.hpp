#ifndef CORETOPOLOGYLEVELSETINITIALIZATION_HPP
#define CORETOPOLOGYLEVELSETINITIALIZATION_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <map>

struct TopologyLevelSetDesign;

class CoreTopologyLevelSetInitialization
{
public:
  /**
   * @brief Resolve zero automatic lengths from positive participating Cubit volumes.
   * @param requested Settings with optional zero automatic lengths.
   * @param element_volumes Complete positive Cubit volume map keyed by global element ID.
   * @param[out] resolved Valid positive resolved lengths, or zero initialization on failure.
   * @param[out] characteristic_length Mean-volume length h, or zero on failure.
   * @return False with empty outputs on invalid settings/volumes or conflicting lengths.
   * @note h=cbrt(mean volume); automatic regularization=1.5h, spacing=6h,
   * radius=spacing/4. Explicit lengths are retained and validated after resolution.
   */
  static bool resolve_settings(const TopologyLevelSetSettings& requested, const std::map<int, double>& element_volumes, TopologyLevelSetSettings& resolved, double& characteristic_length);

  /**
   * @brief Query current mesh nodes and create a deterministic seeded initial field.
   * @param settings Valid requested settings, including zero automatic lengths.
   * @param void_ratio Positive void-to-solid stiffness interpolation floor less than one.
   * @param preserved Complete element preservation flags.
   * @param element_volumes Positive Cubit volumes with exactly the same element IDs.
   * @param[out] design Initial field and resolved metadata; empty on failure.
   * @return False with an empty design on failure; no Core model store is modified.
   * @note Element flags and Cubit volumes must have exactly the same global IDs.
   * All nodes of preserved elements are fixed at +1. A periodic sphere lattice
   * is anchored at the free node nearest the domain bounding-box center; ties
   * use the smaller global node ID. phi=clamp((distance-radius)/radius,-1,1).
   * One free non-anchor node is set to zero if no free nodal coefficient lies
   * in the Heaviside band. For a single free node that node is set to zero.
   * This fallback is recorded; geometry verification must still establish a
   * resolved free interface. Initial volume need not meet the target constraint.
   */
  static bool initialize(const TopologyLevelSetSettings& settings, double void_ratio, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, TopologyLevelSetDesign& design);
};

#endif
