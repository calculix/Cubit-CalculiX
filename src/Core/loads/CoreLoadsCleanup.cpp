#include "Core/loads/CoreLoadsCleanup.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadRemoval.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "CubitInterface.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"

#include <cstddef>

namespace
{
template<typename LoadType>
bool cleanup_set_loads(LoadType& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, const std::string& label, bool nodeset, std::string& log)
{
  CoreMeshQuery query(mesh);
  bool changed = false;

  for (std::size_t index = loads.loads_data.size(); index > 0; index--)
  {
    auto& load = loads.loads_data[index - 1];
    const int load_id = load[0];

    if (load[2] != -1)
    {
      if (!amplitudes.check_amplitude_exists(load[2]))
      {
        log.append("Amplitude ID " + std::to_string(load[2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load " + label + " ID " + std::to_string(load_id) + " will be deleted.\n");
        load[2] = -1;
        changed = true;
      }
    }

    const bool target_exists = nodeset ? query.check_nodeset_exists(load[4]) : query.check_sideset_exists(load[4]);

    if (!target_exists)
    {
      const std::string set_label = nodeset ? "Nodeset" : "Sideset";
      log.append(set_label + " ID " + std::to_string(load[4]) + " doesn't exist.\n");
      log.append(label + " ID " + std::to_string(load_id) + " will be deleted.\n");

      CoreLoadRemoval::delete_load(loads, steps, load_id);
      changed = true;
    }
  }

  return changed;
}

template<typename LoadType>
bool cleanup_block_loads(LoadType& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, const std::string& label, std::string& log)
{
  bool changed = false;

  for (std::size_t index = loads.loads_data.size(); index > 0; index--)
  {
    auto& load = loads.loads_data[index - 1];
    const int load_id = load[0];

    if (load[2] != -1)
    {
      if (!amplitudes.check_amplitude_exists(load[2]))
      {
        log.append("Amplitude ID " + std::to_string(load[2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load " + label + " ID " + std::to_string(load_id) + " will be deleted.\n");
        load[2] = -1;
        changed = true;
      }
    }

    if (!blocks.check_block_exists(load[4]))
    {
      log.append("Block ID " + std::to_string(load[4]) + " doesn't exist.\n");
      log.append(label + " ID " + std::to_string(load_id) + " will be deleted.\n");

      CoreLoadRemoval::delete_load(loads, steps, load_id);
      changed = true;
    }
  }

  return changed;
}

template<typename SubloadType>
bool invalid_trajectory_geometry(SubloadType& loads, int subload_id, int trajectory_id, std::string& log)
{
  bool invalid = false;

  const auto missing_reference = [&](const std::string& message)
  {
    log.append(message);
    log.append("Trajectory ID " + std::to_string(trajectory_id) + " will be deleted.\n");
    invalid = true;
  };

  const int index = loads.get_loads_data_id_from_load_id(subload_id);

  if (index == -1)
  {
    missing_reference("Trajectory subload ID " + std::to_string(subload_id) + " doesn't exist.\n");
    return true;
  }

  const auto& load = loads.loads_data[index];
  const int curve_id = load[2];
  const int vertex_id = load[3];

  if (!CoreMeshQuery::check_curve_exists(curve_id))
  {
    missing_reference("Curve ID " + std::to_string(curve_id) + " doesn't exist.\n");
  }

  if (!CoreMeshQuery::check_vertex_exists(vertex_id))
  {
    missing_reference("Vertex ID " + std::to_string(vertex_id) + " doesn't exist.\n");
  }

  const auto vertices = CubitInterface::parse_cubit_list("vertex", std::to_string(vertex_id) + " in curve " + std::to_string(curve_id));

  if (vertices.empty())
  {
    missing_reference("Vertex ID " + std::to_string(vertex_id) + " doesn't exist in Curve.\n");
  }

  return invalid;
}

template<typename LoadType>
bool cleanup_thermal_surface_loads(LoadType& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, const std::string& label, std::string& log)
{
  CoreMeshQuery query(mesh);
  bool changed = false;

  for (std::size_t index = loads.loads_data.size(); index > 0; index--)
  {
    auto& load = loads.loads_data[index - 1];
    const int load_id = load[0];

    const auto clear_missing_amplitude = [&](std::size_t column, const std::string& prefix)
    {
      if (load[column] != -1)
      {
        if (!amplitudes.check_amplitude_exists(load[column]))
        {
          log.append(prefix + "Amplitude ID " + std::to_string(load[column]) + " doesn't exist.\n");
          log.append(prefix + "Amplitude Reference from Load " + label + " ID " + std::to_string(load_id) + " will be deleted.\n");
          load[column] = -1;
          changed = true;
        }
      }
    };

    clear_missing_amplitude(2, "");
    clear_missing_amplitude(7, label + " ");

    if (!query.check_sideset_exists(load[4]))
    {
      log.append("Sideset ID " + std::to_string(load[4]) + " doesn't exist.\n");
      log.append(label + " ID " + std::to_string(load_id) + " will be deleted.\n");

      CoreLoadRemoval::delete_load(loads, steps, load_id);
      changed = true;
    }
  }

  return changed;
}
}

bool CoreLoadsCleanup::cleanup(CoreLoadsForces& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_set_loads(loads, amplitudes, steps, mesh, "Force", true, log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsPressures& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_set_loads(loads, amplitudes, steps, mesh, "Pressure", false, log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsHeatfluxes& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_set_loads(loads, amplitudes, steps, mesh, "Heatflux", false, log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsGravity& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, std::string& log)
{
  return cleanup_block_loads(loads, amplitudes, steps, blocks, "Gravity", log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsCentrifugal& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, std::string& log)
{
  return cleanup_block_loads(loads, amplitudes, steps, blocks, "Centrifugal", log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsTrajectory& loads, CoreSteps& steps, std::string& log)
{
  bool changed = false;

  for (std::size_t index = loads.loads_data.size(); index > 0; index--)
  {
    const auto& load = loads.loads_data[index - 1];
    const int load_id = load[0];
    bool remove_load = false;

    if (load[1] == 1)
    {
      remove_load = invalid_trajectory_geometry(*loads.heatflux, load[2], load_id, log);
    }
    else if (load[1] == 2)
    {
      remove_load = invalid_trajectory_geometry(*loads.bodyheatfluxsphere, load[2], load_id, log);
    }

    if (remove_load)
    {
      CoreLoadRemoval::delete_load(loads, steps, load_id);
      changed = true;
    }
  }

  return changed;
}

bool CoreLoadsCleanup::cleanup(CoreLoadsFilm& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_thermal_surface_loads(loads, amplitudes, steps, mesh, "Film", log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsRadiation& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_thermal_surface_loads(loads, amplitudes, steps, mesh, "Radiation", log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsSurfaceTraction& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_set_loads(loads, amplitudes, steps, mesh, "Surface Traction", false, log);
}

bool CoreLoadsCleanup::cleanup(CoreLoadsBodyHeatflux& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, std::string& log)
{
  bool changed = false;

  for (std::size_t index = loads.loads_data.size(); index > 0; index--)
  {
    auto& load = loads.loads_data[index - 1];
    const int load_id = load[0];

    if (load[2] != -1)
    {
      if (!amplitudes.check_amplitude_exists(load[2]))
      {
        log.append("Amplitude ID " + std::to_string(load[2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load BodyHeatflux ID " + std::to_string(load_id) + " will be deleted.\n");
        load[2] = -1;
        changed = true;
      }
    }

    bool remove_load = false;

    if (load[5] == 1 || load[5] == 2)
    {
      const auto elements = loads.get_elements_from_element_id(load[4]);

      // Entry zero stores the selection ID.
      for (std::size_t element_index = 1; element_index < elements.size(); element_index++)
      {
        const int element_id = elements[element_index];
        const bool exists = load[5] == 1 ? blocks.check_block_exists(element_id) : CoreMeshQuery::check_global_element_exists(element_id);

        if (!exists)
        {
          if (load[5] == 1)
          {
            log.append("Block ID " + std::to_string(element_id) + " doesn't exist.\n");
          }
          else
          {
            log.append("Global Element ID " + std::to_string(element_id) + " doesn't exist.\n");
            log.append("Check if assigned Elements are part of an existing Block.\n");
          }

          log.append("Bodyheatflux ID " + std::to_string(load_id) + " will be deleted.\n");
          remove_load = true;
          break;
        }
      }
    }

    if (remove_load)
    {
      CoreLoadRemoval::delete_load(loads, steps, load_id);
      changed = true;
    }
  }

  return changed;
}

bool CoreLoadsCleanup::cleanup(const CoreLoadsCleanupData& data, std::string& log)
{
  bool changed = false;

  if (cleanup(data.forces, data.amplitudes, data.steps, data.mesh, log))
  {
    changed = true;
  }

  if (cleanup(data.pressures, data.amplitudes, data.steps, data.mesh, log))
  {
    changed = true;
  }

  if (cleanup(data.heatfluxes, data.amplitudes, data.steps, data.mesh, log))
  {
    changed = true;
  }

  if (cleanup(data.gravity, data.amplitudes, data.steps, data.blocks, log))
  {
    changed = true;
  }

  if (cleanup(data.centrifugal, data.amplitudes, data.steps, data.blocks, log))
  {
    changed = true;
  }

  if (cleanup(data.trajectory, data.steps, log))
  {
    changed = true;
  }

  if (cleanup(data.film, data.amplitudes, data.steps, data.mesh, log))
  {
    changed = true;
  }

  if (cleanup(data.radiation, data.amplitudes, data.steps, data.mesh, log))
  {
    changed = true;
  }

  if (cleanup(data.traction, data.amplitudes, data.steps, data.mesh, log))
  {
    changed = true;
  }

  if (cleanup(data.bodyheatflux, data.amplitudes, data.steps, data.blocks, log))
  {
    changed = true;
  }

  return changed;
}
