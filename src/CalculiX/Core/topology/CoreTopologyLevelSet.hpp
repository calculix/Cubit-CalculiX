#ifndef CORETOPOLOGYLEVELSET_HPP
#define CORETOPOLOGYLEVELSET_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <map>

struct TopologyLevelSetDesign;

/** @brief Level-set configuration foundation; no second topology model is owned. */
class CoreTopologyLevelSet
{
public:
  /**
   * @brief Build an initial field, fractions and requested stiffness factors.
   * @return False with empty outputs on failure; no model/material/job store changes.
   * @note Volumes must be queried from Cubit. The caller must keep the mesh
   * unchanged. Designs and captured settings are persisted without a format version. Internal controller wiring exists,
   * but public level-set execution remains disabled.
   */
  static bool build_initial_design(const TopologyLevelSetSettings& settings, double void_ratio, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, TopologyLevelSetDesign& design, std::map<int, double>& factors);

  /// Authoritative defaults, including explicit geometry quadrature controls.
  static TopologyLevelSetSettings default_settings();
  /// Internal QP/backtracking limits; these are not GUI or command defaults.
  static TopologyLevelSetUpdateControls default_update_controls();
  /// False with a Cubit error for non-finite, out-of-range or conflicting settings.
  static bool validate_settings(const TopologyLevelSetSettings& settings);
};

#endif
