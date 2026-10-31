#include "CoreDrawBCData.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/mesh/CoreMeshGeometry.hpp"
#include "MeshExportInterface.hpp"

std::vector<std::vector<double>> CoreDrawBCData::get_draw_data_for_bc_displacement(
  CoreBCsDisplacements& bcs, MeshExportInterface& mesh, int id)
{
  std::vector<std::vector<double>> draw_data;
  int bcs_data_id = bcs.get_bcs_data_id_from_bc_id(id);

  if (bcs_data_id == -1 || bcs.bcs_data[bcs_data_id].size() < 7)
  {
    return draw_data;
  }

  int nodeset_id = bcs.bcs_data[bcs_data_id][4];
  int displacement_id = bcs.bcs_data[bcs_data_id][5];
  int mode_data_id = bcs.get_mode_data_id_from_displacement_id(displacement_id);

  NodesetHandle nodeset;
  if (!mesh.get_nodeset_handle(nodeset_id, nodeset) || mode_data_id == -1)
  {
    return draw_data;
  }

  if (bcs.mode_data[mode_data_id].size() < 7)
  {
    return draw_data;
  }

  for (int dof = 1; dof <= 6; ++dof)
  {
    int mode = bcs.mode_data[mode_data_id][dof];

    if (mode < 0 || mode > 2)
    {
      return draw_data;
    }
  }

  std::vector<std::vector<double>> coords = CoreMeshGeometry::get_nodeset_entities_coords(nodeset_id);

  for (const auto& coord : coords)
  {
    for (int dof = 1; dof <= 6; ++dof)
    {
      if (bcs.mode_data[mode_data_id][dof] != 0)
      {
        draw_data.push_back({coord[0], coord[1], coord[2], static_cast<double>(dof)});
      }
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CoreDrawBCData::get_draw_data_for_bc_temperature(
  CoreBCsTemperatures& bcs, MeshExportInterface& mesh, int id)
{
  std::vector<std::vector<double>> draw_data;
  int bcs_data_id = bcs.get_bcs_data_id_from_bc_id(id);

  if (bcs_data_id == -1 || bcs.bcs_data[bcs_data_id].size() < 8)
  {
    return draw_data;
  }

  int nodeset_id = bcs.bcs_data[bcs_data_id][4];
  NodesetHandle nodeset;

  if (!mesh.get_nodeset_handle(nodeset_id, nodeset))
  {
    return draw_data;
  }

  std::vector<std::vector<double>> coords = CoreMeshGeometry::get_nodeset_entities_coords(nodeset_id);

  for (const auto& coord : coords)
  {
    draw_data.push_back({coord[0], coord[1], coord[2], 11.0});
  }

  return draw_data;
}
