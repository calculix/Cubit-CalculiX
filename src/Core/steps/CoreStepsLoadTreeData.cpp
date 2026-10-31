#include "CoreStepsLoadTreeData.hpp"
#include "CoreSteps.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "CubitMessage.hpp"

#include <cstddef>

namespace
{
  template<typename LoadType>
  std::vector<std::vector<std::string>> get_step_load_tree_data(
    CoreSteps& steps, LoadType& loads, int step_id, int load_type,
    std::size_t name_column, const std::string& prefix)
  {
    std::vector<std::vector<std::string>> tree_data;
    int step_data_id = steps.get_steps_data_id_from_step_id(step_id);

    if (step_data_id == -1)
    {
      return tree_data;
    }

    std::vector<int> data_ids = steps.get_load_data_ids_from_loads_id(steps.steps_data[step_data_id][5]);

    for (int data_id : data_ids)
    {
      const auto& reference = steps.loads_data[data_id];

      if (reference[1] != load_type)
      {
        continue;
      }

      int load_id = reference[2];
      int load_data_id = loads.get_loads_data_id_from_load_id(load_id);

      if (load_data_id < 0)
      {
        PRINT_ERROR("Step ID %d references missing Load ID %d (type %d).\n", step_id, load_id, load_type);
        continue;
      }

      int name_data_id = loads.get_name_data_id_from_name_id(loads.loads_data[load_data_id][name_column]);
      std::string name;

      if (name_data_id != -1 && !loads.name_data[name_data_id][1].empty())
      {
        name = loads.name_data[name_data_id][1];
      }
      else
      {
        name = prefix + std::to_string(load_id);
      }

      tree_data.push_back({std::to_string(load_id), name});
    }

    return tree_data;
  }
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadsforces_tree_data(
  CoreSteps& steps, CoreLoadsForces& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 1, 6, "Force_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadspressures_tree_data(
  CoreSteps& steps, CoreLoadsPressures& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 2, 6, "Pressure_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadsheatfluxes_tree_data(
  CoreSteps& steps, CoreLoadsHeatfluxes& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 3, 6, "Heatflux_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadsgravity_tree_data(
  CoreSteps& steps, CoreLoadsGravity& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 4, 7, "Gravity_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadscentrifugal_tree_data(
  CoreSteps& steps, CoreLoadsCentrifugal& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 5, 8, "Centrifugal_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadstrajectory_tree_data(
  CoreSteps& steps, CoreLoadsTrajectory& loads, int step_id)
{
  std::vector<std::vector<std::string>> tree_data;
  int step_data_id = steps.get_steps_data_id_from_step_id(step_id);

  if (step_data_id == -1)
  {
    return tree_data;
  }

  std::vector<int> data_ids = steps.get_load_data_ids_from_loads_id(steps.steps_data[step_data_id][5]);

  for (int data_id : data_ids)
  {
    const auto& reference = steps.loads_data[data_id];

    if (reference[1] != 6)
    {
      continue;
    }

    int load_id = reference[2];
    int load_data_id = loads.get_loads_data_id_from_load_id(load_id);

    if (load_data_id < 0)
    {
      PRINT_ERROR("Step ID %d references missing Trajectory Load ID %d.\n", step_id, load_id);
      continue;
    }

    std::string name = loads.get_name_from_load_id(load_id);

    if (name.empty())
    {
      name = "Trajectory_" + std::to_string(load_id);
    }

    tree_data.push_back({std::to_string(load_id), name});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadsfilm_tree_data(
  CoreSteps& steps, CoreLoadsFilm& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 7, 9, "Film_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadsradiation_tree_data(
  CoreSteps& steps, CoreLoadsRadiation& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 8, 9, "Radiation_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadssurfacetraction_tree_data(
  CoreSteps& steps, CoreLoadsSurfaceTraction& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 9, 6, "SurfaceTraction_");
}

std::vector<std::vector<std::string>> CoreStepsLoadTreeData::get_steps_loadsbodyheatflux_tree_data(
  CoreSteps& steps, CoreLoadsBodyHeatflux& loads, int step_id)
{
  return get_step_load_tree_data(steps, loads, step_id, 10, 7, "BodyHeatflux_");
}
