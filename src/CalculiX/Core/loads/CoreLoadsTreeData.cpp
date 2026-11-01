#include "CoreLoadsTreeData.hpp"
#include "CoreLoadsForces.hpp"
#include "CoreLoadsPressures.hpp"
#include "CoreLoadsHeatfluxes.hpp"
#include "CoreLoadsGravity.hpp"
#include "CoreLoadsCentrifugal.hpp"
#include "CoreLoadsTrajectory.hpp"
#include "CoreLoadsFilm.hpp"
#include "CoreLoadsRadiation.hpp"
#include "CoreLoadsSurfaceTraction.hpp"
#include "CoreLoadsBodyHeatflux.hpp"

#include <cstddef>

namespace
{
  template<typename LoadType>
  std::vector<std::vector<std::string>> get_named_load_tree_data(
    LoadType& loads, const std::string& prefix, std::size_t name_column = 6)
  {
    std::vector<std::vector<std::string>> tree_data;

    for (const auto& load : loads.loads_data)
    {
      int name_data_id = loads.get_name_data_id_from_name_id(load[name_column]);
      std::string name;

      if (name_data_id != -1 && !loads.name_data[name_data_id][1].empty())
      {
        name = loads.name_data[name_data_id][1];
      }
      else
      {
        name = prefix + std::to_string(load[0]);
      }

      tree_data.push_back({std::to_string(load[0]), name});
    }

    return tree_data;
  }
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadsforces_tree_data(CoreLoadsForces& loads)
{
  return get_named_load_tree_data(loads, "Force_");
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadspressures_tree_data(CoreLoadsPressures& loads)
{
  return get_named_load_tree_data(loads, "Pressure_");
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadsheatfluxes_tree_data(CoreLoadsHeatfluxes& loads)
{
  return get_named_load_tree_data(loads, "Heatflux_");
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadsgravity_tree_data(CoreLoadsGravity& loads)
{
  return get_named_load_tree_data(loads, "Gravity_", 7);
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadscentrifugal_tree_data(CoreLoadsCentrifugal& loads)
{
  return get_named_load_tree_data(loads, "Centrifugal_", 8);
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadstrajectory_tree_data(CoreLoadsTrajectory& loads)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& load : loads.loads_data)
  {
    std::string name = loads.get_name_from_load_id(load[0]);

    if (name.empty())
    {
      name = "Trajectory_" + std::to_string(load[0]);
    }

    tree_data.push_back({std::to_string(load[0]), name});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadsfilm_tree_data(CoreLoadsFilm& loads)
{
  return get_named_load_tree_data(loads, "Film_", 9);
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadsradiation_tree_data(CoreLoadsRadiation& loads)
{
  return get_named_load_tree_data(loads, "Radiation_", 9);
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadssurfacetraction_tree_data(CoreLoadsSurfaceTraction& loads)
{
  return get_named_load_tree_data(loads, "SurfaceTraction_", 6);
}

std::vector<std::vector<std::string>> CoreLoadsTreeData::get_loadsbodyheatflux_tree_data(CoreLoadsBodyHeatflux& loads)
{
  return get_named_load_tree_data(loads, "BodyHeatflux_", 7);
}
