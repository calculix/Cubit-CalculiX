#ifndef CORETOPOLOGYGEOMETRY_HPP
#define CORETOPOLOGYGEOMETRY_HPP

#include <array>
#include <map>
#include <set>
#include <vector>

class CoreTopology;
class CoreMaterials;
class CoreBlocks;
class MeshExportInterface;

/** @brief Query the current Cubit mesh for topology geometry and connectivity. */
class CoreTopologyGeometry
{
public:
  /**
   * @brief Query Cubit volumes and optional centers for all prepared elements.
   * @param materials Borrowed store identifying the participating global element IDs.
   * @param blocks Borrowed block/type mapping for Cubit entity resolution.
   * @param topology_id Prepared material collection ID.
   * @param[out] element_volumes Global element ID -> positive volume in model length cubed.
   * @param[out] element_centers Optional global element ID -> finite XYZ center in model lengths.
   * @return True after complete collection; false with cleared outputs on invalid data.
   * @note Supports hex, tet and wedge entities. Queries use type-local Cubit IDs;
   * returned maps retain global element IDs. Neither EVOL nor integration-weight
   * sums supply these volumes. Output maps are cleared on entry and filled only
   * when all queries succeed.
   */
  static bool collect_element_volumes(const CoreMaterials& materials, CoreBlocks& blocks, int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers = nullptr);
  /**
   * @brief Calculate physical quadrature weights for prepared volume elements.
   * @param topology Borrowed case store; the case must exist.
   * @param materials Borrowed store identifying participating global element IDs.
   * @param blocks Borrowed block/type mapping supplying the CalculiX element type.
   * @param topology_id Prepared case/material collection ID.
   * @param[out] weights Global element ID -> one-based CalculiX point ID -> physical weight.
   * @return True for complete valid geometry; false with an empty output otherwise.
   * @note Weights are reference weights times det(J), in model length cubed.
   * Coordinates use expanded Cubit connectivity; ElementIntegration performs the
   * shared local-node-order conversion. Determinants and weights must be positive
   * and finite. Multiply each weight into the adjoint contraction once.
   */
  static bool collect_integration_weights(const CoreTopology& topology, const CoreMaterials& materials, CoreBlocks& blocks, int topology_id, std::map<int, std::map<int, double>>& weights);
  /**
   * @brief Build canonical corner-face keys and their owning volume elements.
   * @param blocks Borrowed block/type mapping.
   * @param mesh Borrowed mesh export interface supplying connectivity.
   * @param[out] face_owners Sorted corner-node-ID vector -> set of global element IDs.
   * @return True after collection; false with empty output for invalid rows.
   * @note Uses hex/tet/wedge corner faces, including for higher-order elements;
   * unsupported entity kinds are skipped. Clears output on entry and commits a
   * local collection on success. Face orientation is not represented by the key.
   */
  static bool collect_face_owners(CoreBlocks& blocks, MeshExportInterface& mesh, std::map<std::vector<int>, std::set<int>>& face_owners);
  /**
   * @brief Append elements incident to any selected expanded-connectivity node.
   * @param blocks Borrowed block/type mapping.
   * @param mesh Borrowed mesh export interface supplying connectivity.
   * @param nodes Node IDs to match; empty input succeeds without modifying elements.
   * @param[in,out] elements Global element IDs to extend; existing entries are retained.
   * @return True after scanning; false for invalid connectivity rows.
   * @note Includes midside nodes. This appends incrementally: earlier insertions
   * can remain if a later row fails. It does not restrict the union to one case.
   */
  static bool append_node_elements(CoreBlocks& blocks, MeshExportInterface& mesh, const std::set<int>& nodes, std::set<int>& elements);
  /**
   * @brief Collect expanded-connectivity nodes of the supplied volume elements.
   * @param blocks Borrowed block/type mapping for Cubit entity resolution.
   * @param element_materials Global element IDs in the keys; material IDs are not used.
   * @param[out] nodes Deduplicated positive node IDs.
   * @return True on complete collection; false with an empty output on invalid data.
   * @note Supports hex, tet and wedge entities. Output is cleared on entry and
   * replaced only after local collection succeeds; the result includes midside nodes.
   */
  static bool collect_element_nodes(CoreBlocks& blocks, const std::map<int, int>& element_materials, std::set<int>& nodes);
};

#endif
