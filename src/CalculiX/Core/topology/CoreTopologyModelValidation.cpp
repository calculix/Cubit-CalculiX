#include "CoreTopologyModelValidation.hpp"
#include "CoreTopology.hpp"
#include "CubitInterface.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include <cmath>
#include "Core/sections/CoreSections.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "CubitMessage.hpp"
#include <set>
#include <sstream>
#include <string>

bool CoreTopologyModelValidation::validate_sections(CoreSections& sections, CoreBlocks& blocks)
{
  const std::set<std::string> standard_volume_types = {
      "C3D4", "C3D6", "C3D8", "C3D10", "C3D15", "C3D20"
  };

  // The energy reader currently covers volume solid sections.
  for (const auto& section : sections.sections_data)
  {
    if (section.size() < 3 || section[1] != 1)
    {
      PRINT_ERROR("The initial compliance evaluator requires volume solid sections.\n");
      return false;
    }

    const int solid_index = sections.get_solid_section_data_id_from_solid_section_id(section[2]);

    if (solid_index < 0 || sections.solid_section_data[solid_index].size() < 2)
    {
      PRINT_ERROR("Missing solid section data for section %d.\n", section[0]);
      return false;
    }

    int block_id = -1;
    std::istringstream block_stream(sections.solid_section_data[solid_index][1]);

    if (!(block_stream >> block_id) || !(block_stream >> std::ws).eof() || block_id <= 0)
    {
      PRINT_ERROR("Invalid block for section %d.\n", section[0]);
      return false;
    }

    const std::string element_type = blocks.get_ccx_element_type_name(block_id);

    if (standard_volume_types.count(element_type) == 0)
    {
      PRINT_ERROR("Topology optimization does not support element type '%s' in block %d.\n", element_type.c_str(), block_id);
      return false;
    }
  }

  return true;
}

bool CoreTopologyModelValidation::validate_steps(CoreSteps& steps, const CoreHBCs& hbcs, std::set<int>& displacement_ids)
{
  displacement_ids.clear();
  std::set<int> collected;

  for (const auto& step : steps.steps_data)
  {
    if (step.size() < 9 || step[3] != 2)
    {
      PRINT_ERROR("The initial compliance evaluator requires static steps.\n");
      return false;
    }

    const int parameter_index = steps.get_parameter_data_id_from_parameter_id(step[2]);

    if (parameter_index < 0 || steps.parameter_data[parameter_index].size() < 3)
    {
      PRINT_ERROR("Missing parameters for step %d.\n", step[0]);
      return false;
    }

    const std::string& nlgeom = steps.parameter_data[parameter_index][2];

    if (!nlgeom.empty() && nlgeom != "NO")
    {
      PRINT_ERROR("Compliance optimization requires NLGEOM=NO in step %d.\n", step[0]);
      return false;
    }

    for (int index : steps.get_load_data_ids_from_loads_id(step[5]))
    {
      const auto& load = steps.loads_data[index];

      // 1 = force, 2 = pressure, 9 = surface traction.
      if (load.size() < 3 || (load[1] != 1 && load[1] != 2 && load[1] != 9))
      {
        PRINT_ERROR("Step %d has a load unsupported by the initial compliance evaluator.\n", step[0]);
        return false;
      }
    }

    for (int index : steps.get_bc_data_ids_from_bcs_id(step[6]))
    {
      const auto& bc = steps.bcs_data[index];

      if (bc.size() < 3 || bc[1] != 1)
      {
        PRINT_ERROR("The initial compliance evaluator supports displacement BCs only, in step %d.\n", step[0]);
        return false;
      }

      collected.insert(bc[2]);
    }
  }

  // These model-level BCs are also exported.
  for (const auto& bc : hbcs.bcs_data)
  {
    if (bc.size() < 3 || bc[1] != 1)
    {
      PRINT_ERROR("The initial compliance evaluator supports displacement model-level BCs only.\n");
      return false;
    }

    collected.insert(bc[2]);
  }

  displacement_ids.swap(collected);
  return true;
}

bool CoreTopologyModelValidation::validate_displacements(CoreBCsDisplacements& displacements, MeshExportInterface& mesh, const std::set<int>& displacement_ids, bool require_zero_displacements)
{
  CoreMeshQuery mesh_query(mesh);

  for (int bc_id : displacement_ids)
  {
    const int index = displacements.get_bcs_data_id_from_bc_id(bc_id);

    if (index < 0 || displacements.bcs_data[index].size() < 7)
    {
      PRINT_ERROR("Missing displacement BC %d.\n", bc_id);
      return false;
    }

    const int nodeset_id = displacements.bcs_data[index][4];
    const int displacement_id = displacements.bcs_data[index][5];

    if (!mesh_query.check_nodeset_exists(nodeset_id))
    {
      PRINT_ERROR("Nodeset %d referenced by displacement BC %d does not exist.\n", nodeset_id, bc_id);
      return false;
    }

    const int displacement_data_id = displacements.get_displacement_data_id_from_displacement_id(displacement_id);
    const int mode_data_id = displacements.get_mode_data_id_from_displacement_id(displacement_id);

    if (displacement_data_id < 0 || mode_data_id < 0)
    {
      PRINT_ERROR("Missing displacement data for BC %d.\n", bc_id);
      return false;
    }

    if (displacements.displacement_data[displacement_data_id].size() < 7 ||
        displacements.mode_data[mode_data_id].size() < 7)
    {
      PRINT_ERROR("Invalid displacement data for BC %d.\n", bc_id);
      return false;
    }

    for (int dof = 1; dof <= 6; ++dof)
    {
      const int mode = displacements.mode_data[mode_data_id][dof];

      if (mode < 0 || mode > 2)
      {
        PRINT_ERROR("Invalid displacement mode for BC %d, DOF %d.\n", bc_id, dof);
        return false;
      }

      if (mode == 1)
      {
        const double magnitude = displacements.displacement_data[displacement_data_id][dof];

        if (!std::isfinite(magnitude))
        {
          PRINT_ERROR("Invalid displacement magnitude for BC %d, DOF %d.\n", bc_id, dof);
          return false;
        }

        if (require_zero_displacements && magnitude != 0.0)
        {
          PRINT_ERROR("The initial compliance evaluator requires zero prescribed displacement in BC %d.\n", bc_id);
          return false;
        }
      }
    }
  }

  return true;
}

bool CoreTopologyModelValidation::validate_model(const CoreTopologyModelValidationData& data, bool require_zero_displacements)
{
  if (data.steps.steps_data.empty())
  {
    PRINT_ERROR("Compliance optimization requires an analysis step.\n");
    return false;
  }

  if (!data.contacts.contactpairs_data.empty() || !data.initialconditions.initialconditions_data.empty())
  {
    PRINT_ERROR("The initial compliance evaluator does not support contact pairs or initial conditions.\n");
    return false;
  }

  if (!data.blocks)
  {
    PRINT_ERROR("Compliance validation interfaces are not initialized.\n");
    return false;
  }

  if (!validate_sections(data.sections, *data.blocks))
  {
    return false;
  }

  std::set<int> displacement_ids;

  if (!validate_steps(data.steps, data.hbcs, displacement_ids))
  {
    return false;
  }

  return validate_displacements(data.displacements, data.mesh, displacement_ids, require_zero_displacements);
}

bool CoreTopologyModelValidation::validate_run(const CoreTopology& topology, MeshExportInterface& mesh, int topology_id, const std::function<bool()>& validate_model)
{
  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto& configuration = entry->second;

  if (configuration.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is already running.\n", topology_id);
    return false;
  }

  // Internal controller wiring exists, but normal runs must not precede native numerical verification.
  if (configuration.method == TopologyMethod::LevelSet)
  {
    PRINT_ERROR("Level-set execution remains disabled pending native numerical verification.\n");
    return false;
  }

  if (configuration.method != TopologyMethod::Density ||
      (configuration.objective != TopologyObjective::Compliance &&
       configuration.objective != TopologyObjective::Displacement))
  {
    PRINT_ERROR("Topology currently supports density optimization with compliance or displacement objectives.\n");
    return false;
  }

  if (!validate_model())
  {
    return false;
  }

  const int nodeset_id = configuration.displacement_nodeset;
  const int component = configuration.displacement_component;

  if (configuration.objective == TopologyObjective::Displacement && nodeset_id <= 0)
  {
    PRINT_ERROR("Displacement optimization requires a displacement nodeset.\n");
    return false;
  }

  if (nodeset_id > 0)
  {
    if (component < 0 || component > 3)
    {
      PRINT_ERROR("Invalid topology displacement component.\n");
      return false;
    }

    if (!CoreMeshQuery(mesh).check_nodeset_exists(nodeset_id) || CubitInterface::get_nodeset_nodes_inclusive(nodeset_id).empty())
    {
      PRINT_ERROR("Topology displacement nodeset %d is missing or empty.\n", nodeset_id);
      return false;
    }
  }

  return true;
}

bool CoreTopologyModelValidation::validate_displacement_selection(const CoreTopology& topology, int topology_id, int iteration_id)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology displacement selection request.\n");
    return false;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto& configuration = entry->second;

  if (configuration.displacement_nodeset <= 0 || configuration.displacement_component < 0 || configuration.displacement_component > 3)
  {
    PRINT_ERROR("A displacement nodeset and valid component are required.\n");
    return false;
  }

  const auto evaluation = configuration.iteration_evaluations.find(iteration_id);

  if (evaluation != configuration.iteration_evaluations.end() && evaluation->second.displacement_available)
  {
    if (evaluation->second.displacement_nodeset != configuration.displacement_nodeset ||
        evaluation->second.displacement_component != configuration.displacement_component)
    {
      PRINT_ERROR("The displacement selection differs from the stored iteration measurement.\n");
      return false;
    }
  }

  return true;
}
