#include "CoreStepBCExport.hpp"
#include "CoreSteps.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitMessage.hpp"

namespace
{
  void append_custom_lines(
    CoreCustomLines& customlines, std::vector<std::string>& lines,
    const std::string& position, const std::string& keyword, int id)
  {
    const auto data = customlines.get_customline_data(position, keyword, id);
    lines.insert(lines.end(), data.begin(), data.end());
  }
}

bool CoreStepBCExport::append(
  CoreSteps& steps, CoreBCsDisplacements& bcs, CoreCustomLines& customlines,
  MeshExportInterface& mesh, const std::vector<int>& assignment_ids,
  std::vector<std::string>& lines)
{
  CoreMeshQuery query(mesh);

  for (int assignment_id : assignment_ids)
  {
    const auto& assignment = steps.bcs_data[assignment_id];

    if (assignment[1] != 1)
    {
      continue;
    }

    const int bc_id = assignment[2];
    const int index = bcs.get_bcs_data_id_from_bc_id(bc_id);

    if (index == -1 || bcs.bcs_data[index].size() < 7)
    {
      PRINT_ERROR("Missing or invalid displacement BC %d.\n", bc_id);
      return false;
    }

    const auto& bc = bcs.bcs_data[index];
    const int displacement_index = bcs.get_displacement_data_id_from_displacement_id(bc[5]);
    const int mode_index = bcs.get_mode_data_id_from_displacement_id(bc[5]);
    const int delay_index = bcs.get_time_delay_data_id_from_time_delay_id(bc[3]);
    const int nodeset_id = bc[4];

    if (displacement_index == -1 || mode_index == -1 || delay_index == -1)
    {
      PRINT_ERROR("Missing displacement data for BC %d.\n", bc_id);
      return false;
    }

    if (bcs.displacement_data[displacement_index].size() < 7 ||
        bcs.mode_data[mode_index].size() < 7 ||
        bcs.time_delay_data[delay_index].size() < 2 ||
        !query.check_nodeset_exists(nodeset_id) ||
        query.get_nodeset_name(nodeset_id).empty())
    {
      PRINT_ERROR("Invalid displacement data for BC %d.\n", bc_id);
      return false;
    }

    bool active = false;

    for (int dof = 1; dof <= 6; ++dof)
    {
      if (bcs.mode_data[mode_index][dof] != 0)
      {
        active = true;
      }
    }

    append_custom_lines(customlines, lines, "BEFORE", "DISPLACEMENT", bc_id);
    const std::string data = bcs.get_bc_export(bc_id);

    if (data.empty() && active)
    {
      PRINT_ERROR("Failed exporting displacement BC %d.\n", bc_id);
      return false;
    }

    if (!data.empty())
    {
      lines.push_back(data);
    }

    append_custom_lines(customlines, lines, "AFTER", "DISPLACEMENT", bc_id);
  }

  return true;
}

bool CoreStepBCExport::append(
  CoreSteps& steps, CoreBCsTemperatures& bcs, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  for (int assignment_id : assignment_ids)
  {
    const auto& assignment = steps.bcs_data[assignment_id];

    if (assignment[1] != 2)
    {
      continue;
    }

    const int bc_id = assignment[2];
    append_custom_lines(customlines, lines, "BEFORE", "TEMPERATURE", bc_id);
    const std::string data = bcs.get_bc_export(bc_id);

    if (data.empty())
    {
      PRINT_ERROR("Failed exporting temperature BC %d.\n", bc_id);
      return false;
    }

    lines.push_back(data);
    append_custom_lines(customlines, lines, "AFTER", "TEMPERATURE", bc_id);
  }

  return true;
}
