#include "Core/CalculiXCore.hpp"
#include "CoreDraw.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::draw_all(double size)
{
  return draw->draw_all(size);
}

bool CalculiXCore::draw_loads(double size)
{
  return draw->draw_loads(size);
}

bool CalculiXCore::draw_bcs(double size)
{
  return draw->draw_bcs(size);
}

bool CalculiXCore::draw_orientations(double size)
{
  return draw->draw_orientations(size);
}

bool CalculiXCore::draw_equations(double size)
{
  return draw->draw_equations(size);
}

bool CalculiXCore::draw_equationgroups(double size)
{
  return draw->draw_equationgroups(size);
}

bool CalculiXCore::draw_load_forces(double size)
{
  return draw->draw_load_forces(size);
}

bool CalculiXCore::draw_load_pressures(double size)
{
  return draw->draw_load_pressures(size);
}

bool CalculiXCore::draw_load_heatfluxes(double size)
{
  return draw->draw_load_heatfluxes(size);
}

bool CalculiXCore::draw_load_gravities(double size)
{
  return draw->draw_load_gravities(size);
}

bool CalculiXCore::draw_load_centrifugals(double size)
{
  return draw->draw_load_centrifugals(size);
}

bool CalculiXCore::draw_load_trajectories(double size)
{
  return draw->draw_load_trajectories(size);
}

bool CalculiXCore::draw_load_films(double size)
{
  return draw->draw_load_films(size);
}

bool CalculiXCore::draw_load_radiations(double size)
{
  return draw->draw_load_radiations(size);
}

bool CalculiXCore::draw_load_surface_tractions(double size)
{
  return draw->draw_load_surface_tractions(size);
}

bool CalculiXCore::draw_load_bodyheatfluxes(double size)
{
  return draw->draw_load_bodyheatfluxes(size);
}

bool CalculiXCore::draw_bc_displacements(double size)
{
  return draw->draw_bc_displacements(size);
}

bool CalculiXCore::draw_bc_temperatures(double size)
{
  return draw->draw_bc_temperatures(size);
}

bool CalculiXCore::draw_load_force(std::vector<int> force_ids, double size)
{
  for (int id : force_ids)
  {
    draw->draw_load_force(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_pressure(std::vector<int> pressure_ids, double size)
{
  for (int id : pressure_ids)
  {
    draw->draw_load_pressure(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_heatflux(std::vector<int> heatflux_ids, double size)
{
  for (int id : heatflux_ids)
  {
    draw->draw_load_heatflux(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_gravity(std::vector<int> gravity_ids, double size)
{
  for (int id : gravity_ids)
  {
    draw->draw_load_gravity(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_centrifugal(std::vector<int> centrifugal_ids, double size)
{
  for (int id : centrifugal_ids)
  {
    draw->draw_load_centrifugal(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_trajectory(std::vector<int> trajectory_ids, double size)
{
  for (int id : trajectory_ids)
  {
    draw->draw_load_trajectory(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_film(std::vector<int> film_ids, double size)
{
  for (int id : film_ids)
  {
    draw->draw_load_film(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_radiation(std::vector<int> radiation_ids, double size)
{
  for (int id : radiation_ids)
  {
    draw->draw_load_radiation(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_surface_traction(std::vector<int> surface_traction_ids, double size)
{
  for (int id : surface_traction_ids)
  {
    draw->draw_load_surface_traction(id, size);
  }

  return true;
}

bool CalculiXCore::draw_load_bodyheatflux(std::vector<int> bodyheatflux_ids, double size)
{
  for (int id : bodyheatflux_ids)
  {
    draw->draw_load_bodyheatflux(id, size);
  }

  return true;
}

bool CalculiXCore::draw_bc_displacement(std::vector<int> displacement_ids, double size)
{
  for (int id : displacement_ids)
  {
    draw->draw_bc_displacement(id, size);
  }

  return true;
}

bool CalculiXCore::draw_bc_temperature(std::vector<int> temperature_ids, double size)
{
  for (int id : temperature_ids)
  {
    draw->draw_bc_temperature(id, size);
  }

  return true;
}

bool CalculiXCore::draw_orientation(std::vector<int> orientation_ids, double size)
{
  for (int id : orientation_ids)
  {
    draw->draw_orientation(id, size);
  }

  return true;
}

bool CalculiXCore::draw_equation(std::vector<int> equation_ids, double size)
{
  for (int id : equation_ids)
  {
    draw->draw_equation(id, size);
  }

  return true;
}

bool CalculiXCore::draw_equationgroup(std::vector<int> equationgroup_ids, double size)
{
  for (int id : equationgroup_ids)
  {
    draw->draw_equationgroup(id, size);
  }

  return true;
}

bool CalculiXCore::draw_topology(int topology_id, int iteration_id, double threshold)
{
  if (!draw)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }

  return draw->draw_topology(topology_id, iteration_id, threshold);
}

bool CalculiXCore::topology_animation_active() const
{
  return draw && draw->topology_animation_active();
}

void CalculiXCore::cancel_topology_animation()
{
  if (draw)
  {
    draw->cancel_topology_animation();
  }
}

bool CalculiXCore::animate_topology(int topology_id, double threshold, int fps, std::string save_filepath, bool overwrite)
{
  if (!draw)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }

  return draw->animate_topology(topology_id, threshold, fps, save_filepath, overwrite);
}
