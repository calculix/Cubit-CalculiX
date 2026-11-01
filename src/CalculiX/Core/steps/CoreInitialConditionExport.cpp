#include "CoreInitialConditionExport.hpp"
#include "CoreInitialConditions.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "ScientificNumber.hpp"
#include "CubitMessage.hpp"

#include <vector>

namespace
{
  std::string format_stress(const std::vector<double>& data, int first_component)
  {
    std::string output;

    for (int i = 0; i < 6; ++i)
    {
      output += "," + ScientificNumber::format(data[first_component + i], 6);
    }

    return output;
  }
}

std::string CoreInitialConditionExport::get_export(
  CoreInitialConditions& conditions, CoreBCsDisplacements& displacements,
  CoreBCsTemperatures& temperatures, CoreCustomLines& customlines,
  CoreBlocks& blocks, MeshExportInterface& mesh)
{
  CoreMeshQuery query(mesh);
  std::vector<std::string> lines = {
    "********************************** I N I T I A L C O N D I T I O N S ****************************"
  };

  const auto append_custom_lines = [&](const std::string& position, int id)
  {
    const auto data = customlines.get_customline_data(position, "INITIALCONDITION", id);
    lines.insert(lines.end(), data.begin(), data.end());
  };

  for (const auto& condition : conditions.initialconditions_data)
  {
    append_custom_lines("BEFORE", condition[0]);

    if (condition[1] == 1)
    {
      const int index = conditions.get_displacement_data_id_from_displacement_id(condition[2]);

      if (index == -1 || conditions.displacement_data[index].size() < 2)
      {
        PRINT_ERROR("Missing initial-displacement reference.\n");
        return {};
      }

      const std::string& reference = conditions.displacement_data[index][1];

      if (!reference.empty())
      {
        const int bc_id = std::stoi(reference);
        const int bc_index = displacements.get_bcs_data_id_from_bc_id(bc_id);

        if (bc_index == -1 || displacements.bcs_data[bc_index].size() < 7)
        {
          PRINT_ERROR("Missing displacement BC %d for initial conditions.\n", bc_id);
          return {};
        }

        const auto& bc = displacements.bcs_data[bc_index];
        const int displacement_index = displacements.get_displacement_data_id_from_displacement_id(bc[5]);
        const int mode_index = displacements.get_mode_data_id_from_displacement_id(bc[5]);
        const int nodeset_id = bc[4];

        if (displacement_index == -1 || mode_index == -1)
        {
          PRINT_ERROR("Missing displacement data for BC %d.\n", bc_id);
          return {};
        }

        if (displacements.displacement_data[displacement_index].size() < 7 ||
            displacements.mode_data[mode_index].size() < 7 ||
            !query.check_nodeset_exists(nodeset_id) ||
            query.get_nodeset_name(nodeset_id).empty())
        {
          PRINT_ERROR("Invalid initial-displacement data for BC %d.\n", bc_id);
          return {};
        }

        bool active = false;

        for (int dof = 1; dof <= 6; ++dof)
        {
          if (displacements.mode_data[mode_index][dof] != 0)
          {
            active = true;
          }
        }

        const std::string data = displacements.get_initial_displacement_export(bc_id);

        if (data.empty() && active)
        {
          PRINT_ERROR("Failed exporting initial displacement from BC %d.\n", bc_id);
          return {};
        }

        if (!data.empty())
        {
          lines.push_back(data);
        }
      }
    }
    else if (condition[1] == 2)
    {
      const int index = conditions.get_temperature_data_id_from_temperature_id(condition[2]);

      if (index == -1 || conditions.temperature_data[index].size() < 2)
      {
        PRINT_ERROR("Missing initial temperature reference.\n");
        return {};
      }

      const std::string& reference = conditions.temperature_data[index][1];

      if (!reference.empty())
      {
        const int bc_id = std::stoi(reference);
        const std::string data = temperatures.get_initial_temperature_export(bc_id);

        if (data.empty())
        {
          PRINT_ERROR("Failed exporting initial temperature from BC %d.\n", bc_id);
          return {};
        }

        lines.push_back(data);
      }
    }
    else if (condition[1] == 3)
    {
      lines.push_back("*INITIAL CONDITIONS,TYPE=STRESS");

      const auto element_data_ids = conditions.get_stress_element_data_ids_from_stress_id(condition[2]);

      for (int data_id : element_data_ids)
      {
        const auto& data = conditions.stress_element_data[data_id];
        const int element_id = static_cast<int>(data[1]);
        const int integration_point = static_cast<int>(data[2]);

        lines.push_back(std::to_string(element_id) + "," + std::to_string(integration_point) + format_stress(data, 3));
      }

      const auto block_data_ids = conditions.get_stress_block_data_ids_from_stress_id(condition[2]);

      for (int data_id : block_data_ids)
      {
        const auto& data = conditions.stress_block_data[data_id];
        const int block_id = static_cast<int>(data[1]);
        const int integration_points = blocks.get_ccx_element_type_integration_points(block_id);
        const auto element_ids = query.get_block_element_ids(blocks, block_id);
        const std::string stress = format_stress(data, 2);

        for (int element_id : element_ids)
        {
          for (int ip = 1; ip <= integration_points; ++ip)
          {
            lines.push_back(std::to_string(element_id) + "," + std::to_string(ip) + stress);
          }
        }
      }
    }

    append_custom_lines("AFTER", condition[0]);
  }

  std::string output;

  for (const auto& line : lines)
  {
    output += line + "\n";
  }

  return output;
}
