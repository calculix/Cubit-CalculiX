#ifndef CORETOPOLOGYMODELPREPARATION_HPP
#define CORETOPOLOGYMODELPREPARATION_HPP

#include <map>
#include <set>
#include <vector>

struct CoreTopologyPreservationData;

class CoreTopology;
class CoreMaterials;
class CoreSections;
class CoreBlocks;
class MeshExportInterface;

class CoreTopologyModelPreparation
{
public:
  static bool initialize(CoreMaterials& materials, CoreSections& sections, CoreBlocks& blocks, MeshExportInterface& mesh, int topology_id, const std::vector<int>& section_ids);
  static bool resolve_preservation(CoreTopology& topology, const CoreMaterials& materials, int topology_id, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements);
  static bool initialize_run_design(CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, const CoreTopologyPreservationData& preservation, int topology_id, const std::vector<int>& section_ids);
  static bool collect_run_sections(CoreSections& sections, CoreBlocks& blocks, std::vector<int>& section_ids, std::set<int>& block_ids);
};

#endif
