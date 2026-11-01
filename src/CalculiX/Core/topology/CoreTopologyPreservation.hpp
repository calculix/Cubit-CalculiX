#ifndef CORETOPOLOGYPRESERVATION_HPP
#define CORETOPOLOGYPRESERVATION_HPP

#include "CoreTopologyStructs.hpp"
#include <map>
#include <set>
#include <vector>

class CoreSteps;
class CoreMaterials;
class CoreBlocks;
class MeshExportInterface;

class CoreTopologyPreservation
{
public:
  static bool get_set_element_ids(const CoreMaterials& materials, CoreBlocks& blocks, MeshExportInterface& mesh, int topology_id, int set_type, int set_id, std::vector<int>& element_ids);
  static bool collect_step_references(CoreSteps& steps, std::map<int, std::set<int>>& used_loads, std::map<int, std::set<int>>& used_bcs);
  static bool build_set_rows(const TopologyCase& configuration, const std::map<int, std::set<int>>& defaults, const std::set<int>& default_elements, std::vector<TopologyPreservedSetInfo>& sets, std::set<int>& direct_elements);
};

#endif
