#include "CoreTopologyModelPreparation.hpp"
#include "CoreTopologyDefaultPreservation.hpp"
#include "CoreTopologyDensity.hpp"
#include "CoreTopology.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitMessage.hpp"
#include <string>

bool CoreTopologyModelPreparation::initialize(CoreMaterials& materials, CoreSections& sections, CoreBlocks& blocks, MeshExportInterface& mesh, int topology_id, const std::vector<int>& section_ids)
{
  if (topology_id <= 0 || section_ids.empty())
  {
    PRINT_ERROR("Invalid topology initialization.\n");
    return false;
  }

  // Reset data belonging to this topology configuration.
  materials.topology_material_data.erase(topology_id);
  sections.topology_solid_sections.erase(topology_id);

  std::vector<int> element_ids;
  std::vector<int> base_material_ids;
  CoreMeshQuery mesh_query(mesh);

  for (int section_id : section_ids)
  {
    const int section_index = sections.get_sections_data_id_from_section_id(section_id);

    if (section_index < 0 || sections.sections_data[section_index][1] != 1)
    {
      PRINT_ERROR("Section %d is not an existing solid section.\n", section_id);
      return false;
    }

    const int solid_index = sections.get_solid_section_data_id_from_solid_section_id(sections.sections_data[section_index][2]);

    if (solid_index < 0)
    {
      PRINT_ERROR("Solid section data missing for section %d.\n", section_id);
      return false;
    }

    const auto& solid = sections.solid_section_data[solid_index];
    const int block_id = std::stoi(solid[1]);
    const int material_id = std::stoi(solid[2]);

    if (blocks.get_blocks_data_id_from_block_id(block_id) < 0)
    {
      PRINT_ERROR("Block %d for section %d does not exist.\n", block_id, section_id);
      return false;
    }

    const auto block_elements = mesh_query.get_block_element_ids(blocks, block_id);

    if (block_elements.empty())
    {
      PRINT_ERROR("Block %d contains no elements.\n", block_id);
      return false;
    }

    for (int element_id : block_elements)
    {
      element_ids.push_back(element_id);
      base_material_ids.push_back(material_id);
    }
  }

  if (!materials.initialize_topology_materials(topology_id, element_ids, base_material_ids))
  {
    PRINT_ERROR("Failed to initialize materials for topology %d.\n", topology_id);
    return false;
  }

  if (!sections.initialize_topology_solid_sections(topology_id, section_ids))
  {
    // Remove only the collection created by this call.
    materials.topology_material_data.erase(topology_id);
    return false;
  }

  PRINT_INFO("Topology %d initialized with %d elements.\n", topology_id, static_cast<int>(element_ids.size()));
  return true;
}

bool CoreTopologyModelPreparation::resolve_preservation(CoreTopology& topology, const CoreMaterials& materials, int topology_id, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements)
{
  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Material collection for topology %d not found.\n", topology_id);
    return false;
  }

  std::vector<int> element_ids;

  for (const auto& assignment : collection->second.element_base_material)
  {
    element_ids.push_back(assignment.first);
  }

  return topology.resolve_preserved_elements(topology_id, element_ids, default_sets, default_elements);
}

bool CoreTopologyModelPreparation::initialize_run_design(CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, const CoreTopologyPreservationData& preservation, int topology_id, const std::vector<int>& section_ids)
{
  if (!preservation.blocks)
  {
    PRINT_ERROR("Topology initialization interfaces are not initialized.\n");
    return false;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (entry->second.method != TopologyMethod::Density ||
      (entry->second.objective != TopologyObjective::Compliance &&
       entry->second.objective != TopologyObjective::Displacement))
  {
    PRINT_ERROR("The topology iteration loop supports density optimization with compliance or displacement objectives.\n");
    return false;
  }

  const auto discard_prepared_data = [&]()
  {
    entry->second.element_preserved.clear();
    materials.topology_material_data.erase(topology_id);
    sections.topology_solid_sections.erase(topology_id);
  };

  if (!initialize(materials, sections, *preservation.blocks, preservation.mesh, topology_id, section_ids))
  {
    discard_prepared_data();
    return false;
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Topology material initialization failed.\n");
    discard_prepared_data();
    return false;
  }

  std::map<int, std::set<int>> default_sets;
  std::set<int> default_elements;

  if (!CoreTopologyDefaultPreservation::collect(preservation, default_sets, default_elements) ||
      !resolve_preservation(topology, materials, topology_id, default_sets, default_elements))
  {
    PRINT_ERROR("Failed to resolve preservation for topology %d.\n", topology_id);
    discard_prepared_data();
    return false;
  }

  std::map<int, double> densities;

  for (const auto& assignment : collection->second.element_base_material)
  {
    densities.emplace(assignment.first, 1.0);
  }

  if (!CoreTopologyDensity::set_densities(topology, materials, *preservation.blocks, topology_id, 1, densities, -1))
  {
    discard_prepared_data();
    return false;
  }

  return true;
}

bool CoreTopologyModelPreparation::collect_run_sections(CoreSections& sections, CoreBlocks& blocks, std::vector<int>& section_ids, std::set<int>& block_ids)
{
  section_ids.clear();
  block_ids.clear();

  for (const auto& section : sections.sections_data)
  {
    if (section.size() < 3)
    {
      PRINT_ERROR("Invalid section data.\n");
      return false;
    }

    if (section[1] != 1)
    {
      continue;
    }

    const int solid_index = sections.get_solid_section_data_id_from_solid_section_id(section[2]);

    if (solid_index < 0)
    {
      PRINT_ERROR("Solid section data missing for section %d.\n", section[0]);
      return false;
    }

    const int block_id = std::stoi(sections.solid_section_data[solid_index][1]);

    if (blocks.get_ccx_element_type_name(block_id).rfind("C3D", 0) == 0)
    {
      section_ids.push_back(section[0]);
      block_ids.insert(block_id);
    }
  }

  if (section_ids.empty())
  {
    PRINT_ERROR("Topology optimization requires solid sections containing volume elements.\n");
    return false;
  }

  return true;
}
