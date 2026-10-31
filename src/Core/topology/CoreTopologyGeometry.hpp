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

class CoreTopologyGeometry
{
public:
  static bool collect_element_volumes(const CoreMaterials& materials, CoreBlocks& blocks, int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers = nullptr);
  static bool collect_integration_weights(const CoreTopology& topology, const CoreMaterials& materials, CoreBlocks& blocks, int topology_id, std::map<int, std::map<int, double>>& weights);
  static bool collect_face_owners(CoreBlocks& blocks, MeshExportInterface& mesh, std::map<std::vector<int>, std::set<int>>& face_owners);
  static bool append_node_elements(CoreBlocks& blocks, MeshExportInterface& mesh, const std::set<int>& nodes, std::set<int>& elements);
  static bool collect_element_nodes(CoreBlocks& blocks, const std::map<int, int>& element_materials, std::set<int>& nodes);
};

#endif
