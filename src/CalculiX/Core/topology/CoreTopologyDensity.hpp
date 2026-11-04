#ifndef CORETOPOLOGYDENSITY_HPP
#define CORETOPOLOGYDENSITY_HPP

#include <map>

class CoreTopology;
class CoreMaterials;
class CoreBlocks;

/** @brief Coordinate density storage across borrowed topology, material and block stores. */
class CoreTopologyDensity
{
public:
  /**
   * @brief Store a new design, grouped applied factors and Cubit volume diagnostics.
   * @param topology Prepared case store; the iteration must not already exist.
   * @param materials Prepared base assignments and destination for applied factors.
   * @param blocks Block/type information for the current Cubit mesh.
   * @param topology_id Existing density case with resolved preservation flags.
   * @param iteration_id Positive, unused design ID before job creation.
   * @param densities Complete global-element-ID map; free values respect the density floor.
   * @param source_iteration -1 for no source diagnostics, otherwise a positive earlier ID.
   * @return True when the design and factors are stored and the target is attainable.
   * @note Preserved densities become one. The design retains requested densities;
   * material grouping determines the stored applied stiffness factors. Handled
   * post-insertion volume/target failures erase the new design, factors, cap and
   * cache entries. This is not a general exception-safe transaction.
   */
  static bool set_densities(CoreTopology& topology, CoreMaterials& materials, CoreBlocks& blocks, int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration);
};

#endif
