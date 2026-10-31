#include "CoreLoadsQuery.hpp"
#include "CoreLoadsForces.hpp"
#include "CoreLoadsPressures.hpp"
#include "CoreLoadsHeatfluxes.hpp"
#include "CoreLoadsGravity.hpp"
#include "CoreLoadsCentrifugal.hpp"
#include "CoreLoadsFilm.hpp"
#include "CoreLoadsRadiation.hpp"
#include "CoreLoadsSurfaceTraction.hpp"
#include "CoreLoadsTrajectory.hpp"
#include "CoreLoadsTrajectoryHeatflux.hpp"
#include "CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "CoreLoadsBodyHeatflux.hpp"
#include "CubitInterface.hpp"
#include <cstddef>

namespace
{
  template<typename TrajectoryType>
  std::vector<std::vector<std::string>> get_trajectory_entities(TrajectoryType& loads, int load_id)
  {
    const int index = loads.get_loads_data_id_from_load_id(load_id);

    if (index == -1)
    {
      return {};
    }

    const auto& data = loads.loads_data[index];
    std::vector<std::vector<std::string>> entities = {
      {"curve", std::to_string(data[2])},
      {"vertex", std::to_string(data[3])}
    };

    const auto surface_ids = loads.get_fire_ray_surface_ids_from_fire_ray_surface_id(data[4]);

    for (int surface_id : surface_ids)
    {
      entities.push_back({"surface", std::to_string(surface_id)});
    }

    return entities;
  }

  template<typename LoadType>
  std::vector<std::vector<std::string>> get_target_entities(LoadType& loads, int load_id, const std::string& entity)
  {
    const int index = loads.get_loads_data_id_from_load_id(load_id);

    if (index == -1)
    {
      return {};
    }

    return {{entity, std::to_string(loads.loads_data[index][4])}};
  }
}

bool CoreLoadsQuery::load_exists(const CoreLoadsQueryData& data, int load_type, int load_id)
{
  switch (load_type)
  {
    case 1: return data.forces->get_loads_data_id_from_load_id(load_id) != -1;
    case 2: return data.pressures->get_loads_data_id_from_load_id(load_id) != -1;
    case 3: return data.heatfluxes->get_loads_data_id_from_load_id(load_id) != -1;
    case 4: return data.gravity->get_loads_data_id_from_load_id(load_id) != -1;
    case 5: return data.centrifugal->get_loads_data_id_from_load_id(load_id) != -1;
    case 6: return data.trajectory->get_loads_data_id_from_load_id(load_id) != -1;
    case 7: return data.film->get_loads_data_id_from_load_id(load_id) != -1;
    case 8: return data.radiation->get_loads_data_id_from_load_id(load_id) != -1;
    case 9: return data.traction->get_loads_data_id_from_load_id(load_id) != -1;
    case 10: return data.bodyheatflux->get_loads_data_id_from_load_id(load_id) != -1;
    default: return false;
  }
}

std::vector<int> CoreLoadsQuery::get_load_ids(const CoreLoadsQueryData& data, const std::string& parse_type)
{
  if (parse_type == "loadsforces")
  {
    return data.forces->get_load_ids();
  }

  if (parse_type == "loadspressures")
  {
    return data.pressures->get_load_ids();
  }

  if (parse_type == "loadsheatfluxes")
  {
    return data.heatfluxes->get_load_ids();
  }

  if (parse_type == "loadsgravity")
  {
    return data.gravity->get_load_ids();
  }

  if (parse_type == "loadscentrifugal")
  {
    return data.centrifugal->get_load_ids();
  }

  if (parse_type == "loadstrajectory")
  {
    return data.trajectory->get_load_ids();
  }

  if (parse_type == "loadsfilm")
  {
    return data.film->get_load_ids();
  }

  if (parse_type == "loadsradiation")
  {
    return data.radiation->get_load_ids();
  }

  if (parse_type == "loadssurfacetraction")
  {
    return data.traction->get_load_ids();
  }

  if (parse_type == "loadsbodyheatflux")
  {
    return data.bodyheatflux->get_load_ids();
  }

  return {};
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(
  const CoreLoadsQueryData& data, const std::string& entity, int load_id)
{
  if (entity == "loadsforce")
  {
    return get_entities(*data.forces, load_id);
  }

  if (entity == "loadspressure")
  {
    return get_entities(*data.pressures, load_id);
  }

  if (entity == "loadsheatflux")
  {
    return get_entities(*data.heatfluxes, load_id);
  }

  if (entity == "loadsgravity")
  {
    return get_entities(*data.gravity, load_id);
  }

  if (entity == "loadscentrifugal")
  {
    return get_entities(*data.centrifugal, load_id);
  }

  if (entity == "loadstrajectory")
  {
    return get_entities(*data.trajectory, load_id);
  }

  if (entity == "loadsfilm")
  {
    return get_entities(*data.film, load_id);
  }

  if (entity == "loadsradiation")
  {
    return get_entities(*data.radiation, load_id);
  }

  if (entity == "loadssurfacetraction")
  {
    return get_entities(*data.traction, load_id);
  }

  if (entity == "loadsbodyheatflux")
  {
    return get_entities(*data.bodyheatflux, load_id);
  }

  return {};
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsForces& loads, int load_id)
{
  return get_target_entities(loads, load_id, "nodeset");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsPressures& loads, int load_id)
{
  return get_target_entities(loads, load_id, "sideset");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsHeatfluxes& loads, int load_id)
{
  return get_target_entities(loads, load_id, "sideset");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsGravity& loads, int load_id)
{
  return get_target_entities(loads, load_id, "block");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsCentrifugal& loads, int load_id)
{
  return get_target_entities(loads, load_id, "block");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsFilm& loads, int load_id)
{
  return get_target_entities(loads, load_id, "sideset");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsRadiation& loads, int load_id)
{
  return get_target_entities(loads, load_id, "sideset");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsSurfaceTraction& loads, int load_id)
{
  return get_target_entities(loads, load_id, "sideset");
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsTrajectory& loads, int load_id)
{
  const int index = loads.get_loads_data_id_from_load_id(load_id);

  if (index == -1)
  {
    return {};
  }

  const int subtype_id = loads.loads_data[index][2];
  const std::string type = loads.get_load_type(load_id);

  if (type == "HEATFLUX")
  {
    return get_trajectory_entities(*loads.heatflux, subtype_id);
  }

  if (type == "BODYHEATFLUXSPHERE")
  {
    return get_trajectory_entities(*loads.bodyheatfluxsphere, subtype_id);
  }

  return {};
}

std::vector<std::vector<std::string>> CoreLoadsQuery::get_entities(CoreLoadsBodyHeatflux& loads, int load_id)
{
  const int index = loads.get_loads_data_id_from_load_id(load_id);

  if (index == -1)
  {
    return {};
  }

  const auto& data = loads.loads_data[index];
  const int mode = data[5];

  if (mode != 1 && mode != 2)
  {
    return {};
  }

  std::vector<std::vector<std::string>> entities;
  const auto elements = loads.get_elements_from_element_id(data[4]);

  for (std::size_t i = 1; i < elements.size(); i++)
  {
    const int id = elements[i];
    const std::string entity = mode == 1 ? "block" : CubitInterface::get_element_type(id);
    entities.push_back({entity, std::to_string(id)});
  }

  return entities;
}
