#include "CoreDrawLoadData.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "ScientificNumber.hpp"
#include "Core/mesh/CoreMeshGeometry.hpp"
#include "MeshExportInterface.hpp"

namespace
{
  std::vector<std::vector<double>> get_matching_sideset_draw_data(
    const std::vector<std::vector<int>>& loads_data, int id)
  {
    std::vector<std::vector<double>> draw_data;

    for (const auto& load : loads_data)
    {
      if (load[0] == id)
      {
        std::vector<std::vector<double>> coords_normals = CoreMeshGeometry::get_sideset_entities_coords_normals(load[4]);
        draw_data.insert(draw_data.end(), coords_normals.begin(), coords_normals.end());
      }
    }

    return draw_data;
  }

  std::vector<std::vector<double>> get_sideset_draw_data(MeshExportInterface& mesh, int sideset_id)
  {
    SidesetHandle sideset;

    if (!mesh.get_sideset_handle(sideset_id, sideset))
    {
      return {};
    }

    return CoreMeshGeometry::get_sideset_entities_coords_normals(sideset_id);
  }
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_force(
  CoreLoadsForces& loads, MeshExportInterface& mesh, int id)
{
  int loads_data_id = loads.get_loads_data_id_from_load_id(id);

  if (loads_data_id == -1)
  {
    return {};
  }

  int force_data_id = loads.get_force_data_id_from_force_id(loads.loads_data[loads_data_id][5]);

  if (force_data_id == -1)
  {
    return {};
  }

  int nodeset_id = loads.loads_data[loads_data_id][4];
  NodesetHandle nodeset;

  if (!mesh.get_nodeset_handle(nodeset_id, nodeset))
  {
    return {};
  }

  std::vector<std::vector<double>> draw_data;
  std::vector<std::vector<double>> coords = CoreMeshGeometry::get_nodeset_entities_coords(nodeset_id);
  const auto& force = loads.force_data[force_data_id];

  for (const auto& coord : coords)
  {
    draw_data.push_back({coord[0], coord[1], coord[2], force[1], force[2], force[3]});
    draw_data.push_back({coord[0], coord[1], coord[2], force[4], force[5], force[6]});
  }

  return draw_data;
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_pressure(
  CoreLoadsPressures& loads, MeshExportInterface& mesh, int id)
{
  int loads_data_id = loads.get_loads_data_id_from_load_id(id);

  if (loads_data_id == -1)
  {
    return {};
  }

  return get_sideset_draw_data(mesh, loads.loads_data[loads_data_id][4]);
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_heatflux(
  CoreLoadsHeatfluxes& loads, MeshExportInterface& mesh, int id)
{
  int loads_data_id = loads.get_loads_data_id_from_load_id(id);

  if (loads_data_id == -1)
  {
    return {};
  }

  return get_sideset_draw_data(mesh, loads.loads_data[loads_data_id][4]);
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_film(CoreLoadsFilm& loads, int id)
{
  return get_matching_sideset_draw_data(loads.loads_data, id);
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_radiation(CoreLoadsRadiation& loads, int id)
{
  return get_matching_sideset_draw_data(loads.loads_data, id);
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_surface_traction(
  CoreLoadsSurfaceTraction& loads, int id)
{
  std::vector<std::vector<double>> draw_data;

  for (const auto& load : loads.loads_data)
  {
    if (load[0] == id)
    {
      std::vector<std::vector<double>> coords_normals = CoreMeshGeometry::get_sideset_entities_coords_normals(load[4]);
      int force_data_id = loads.get_force_data_id_from_force_id(load[5]);

      for (const auto& coord : coords_normals)
      {
        const auto& force = loads.force_data[force_data_id];
        draw_data.push_back({coord[0], coord[1], coord[2], force[1], force[2], force[3]});
      }
    }
  }

  return draw_data;
}

std::vector<std::vector<int>> CoreDrawLoadData::get_draw_data_for_load_bodyheatflux(CoreLoadsBodyHeatflux& loads, int id)
{
  std::vector<std::vector<int>> draw_data;

  for (const auto& load : loads.loads_data)
  {
    if (load[0] == id)
    {
      std::vector<int> block_ids;
      std::vector<int> element_ids;

      if (load[5] == 1 || load[5] == 2)
      {
        std::vector<int> elements = loads.get_elements_from_element_id(load[4]);

        for (size_t i = 1; i < elements.size(); ++i)
        {
          if (load[5] == 1)
          {
            block_ids.push_back(elements[i]);
          }
          else
          {
            element_ids.push_back(elements[i]);
          }
        }
      }

      draw_data.push_back(block_ids);
      draw_data.push_back(element_ids);
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_gravity(CoreLoadsGravity& loads, int id)
{
  std::vector<std::vector<double>> draw_data;

  for (const auto& load : loads.loads_data)
  {
    if (load[0] == id)
    {
      int direction_data_id = loads.get_direction_data_id_from_direction_id(load[5]);
      const auto& direction = loads.direction_data[direction_data_id];

      draw_data.push_back({
        0.0, 0.0, 0.0,
        ScientificNumber::parse_with_precision(direction[1]),
        ScientificNumber::parse_with_precision(direction[2]),
        ScientificNumber::parse_with_precision(direction[3])
      });
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CoreDrawLoadData::get_draw_data_for_load_centrifugal(CoreLoadsCentrifugal& loads, int id)
{
  std::vector<std::vector<double>> draw_data;

  for (const auto& load : loads.loads_data)
  {
    if (load[0] == id)
    {
      int direction_data_id = loads.get_direction_data_id_from_direction_id(load[5]);
      int coordinate_data_id = loads.get_coordinate_data_id_from_coordinate_id(load[7]);
      const auto& direction = loads.direction_data[direction_data_id];
      const auto& coordinate = loads.coordinate_data[coordinate_data_id];

      draw_data.push_back({
        ScientificNumber::parse_with_precision(coordinate[1]),
        ScientificNumber::parse_with_precision(coordinate[2]),
        ScientificNumber::parse_with_precision(coordinate[3]),
        ScientificNumber::parse_with_precision(direction[1]),
        ScientificNumber::parse_with_precision(direction[2]),
        ScientificNumber::parse_with_precision(direction[3])
      });
    }
  }

  return draw_data;
}
