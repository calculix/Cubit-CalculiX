#include "CoreHBCExport.hpp"
#include "CoreHBCs.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitMessage.hpp"

#include <vector>

std::string CoreHBCExport::get_export(
  CoreHBCs& hbcs, CoreBCsDisplacements& displacements,
  CoreBCsTemperatures& temperatures, CoreCustomLines& customlines,
  MeshExportInterface& mesh)
{
  CoreMeshQuery query(mesh);
  std::vector<std::string> lines = {
    "********************************** H B C s ****************************"
  };

  const auto append_custom_lines = [&](const std::string& position, const std::string& keyword, int id)
  {
    const auto data = customlines.get_customline_data(position, keyword, id);
    lines.insert(lines.end(), data.begin(), data.end());
  };

  const auto assignment_ids = hbcs.get_bc_data_ids_from_bcs_id(0);

  for (int assignment_id : assignment_ids)
  {
    const auto& assignment = hbcs.bcs_data[assignment_id];

    if (assignment[1] != 1)
    {
      continue;
    }

    const int bc_id = assignment[2];
    const int index = displacements.get_bcs_data_id_from_bc_id(bc_id);

    if (index == -1 || displacements.bcs_data[index].size() < 7)
    {
      PRINT_ERROR("Missing or invalid displacement BC %d.\n", bc_id);
      return {};
    }

    const auto& bc = displacements.bcs_data[index];
    const int displacement_index = displacements.get_displacement_data_id_from_displacement_id(bc[5]);
    const int mode_index = displacements.get_mode_data_id_from_displacement_id(bc[5]);
    const int delay_index = displacements.get_time_delay_data_id_from_time_delay_id(bc[3]);
    const int nodeset_id = bc[4];

    if (displacement_index == -1 || mode_index == -1 || delay_index == -1)
    {
      PRINT_ERROR("Missing displacement data for BC %d.\n", bc_id);
      return {};
    }

    if (displacements.displacement_data[displacement_index].size() < 7 ||
        displacements.mode_data[mode_index].size() < 7 ||
        displacements.time_delay_data[delay_index].size() < 2 ||
        !query.check_nodeset_exists(nodeset_id) ||
        query.get_nodeset_name(nodeset_id).empty())
    {
      PRINT_ERROR("Invalid displacement data for BC %d.\n", bc_id);
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

    append_custom_lines("BEFORE", "DISPLACEMENT", bc_id);
    const std::string data = displacements.get_hbc_export(bc_id);

    if (data.empty() && active)
    {
      PRINT_ERROR("Failed exporting displacement BC %d.\n", bc_id);
      return {};
    }

    if (!data.empty())
    {
      lines.push_back(data);
    }

    append_custom_lines("AFTER", "DISPLACEMENT", bc_id);
  }

  for (int assignment_id : assignment_ids)
  {
    const auto& assignment = hbcs.bcs_data[assignment_id];

    if (assignment[1] != 2)
    {
      continue;
    }

    const int bc_id = assignment[2];
    append_custom_lines("BEFORE", "TEMPERATURE", bc_id);
    const std::string data = temperatures.get_hbc_export(bc_id);

    if (data.empty())
    {
      PRINT_ERROR("Failed exporting temperature BC %d.\n", bc_id);
      return {};
    }

    lines.push_back(data);
    append_custom_lines("AFTER", "TEMPERATURE", bc_id);
  }

  std::string output;

  for (const auto& line : lines)
  {
    output += line + "\n";
  }

  return output;
}
