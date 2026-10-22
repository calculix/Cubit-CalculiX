#include "CalculiXDrawInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_force(int id)
{
  return ccx_core.get_draw_data_for_load_force(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_pressure(int id)
{
  return ccx_core.get_draw_data_for_load_pressure(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_heatflux(int id)
{
  return ccx_core.get_draw_data_for_load_heatflux(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_gravity(int id)
{
  return ccx_core.get_draw_data_for_load_gravity(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_centrifugal(int id)
{
  return ccx_core.get_draw_data_for_load_centrifugal(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_film(int id)
{
  return ccx_core.get_draw_data_for_load_film(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_radiation(int id)
{
  return ccx_core.get_draw_data_for_load_radiation(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_surface_traction(int id)
{
  return ccx_core.get_draw_data_for_load_surface_traction(id);
}

std::vector<std::vector<int>> CalculiXDrawInterface::get_draw_data_for_load_bodyheatflux(int id)
{
  return ccx_core.get_draw_data_for_load_bodyheatflux(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_bc_displacement(int id) // returns coord(3) and dof
{
  return ccx_core.get_draw_data_for_bc_displacement(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_bc_temperature(int id) // returns coord(3) and dof
{
  return ccx_core.get_draw_data_for_bc_temperature(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_orientation(int id)
{
  return ccx_core.get_draw_data_for_orientation(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_equation(int id)
{
  return ccx_core.get_draw_data_for_equation(id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_equationgroup(int id)
{
  return ccx_core.get_draw_data_for_equationgroup(id);
}

bool CalculiXDrawInterface::draw_all(double size) // draw all bc,loads,orientations, equations
{
  return ccx_core.draw_all(size);
}

bool CalculiXDrawInterface::draw_load_force(std::vector<int> force_ids,double size)
{
  return ccx_core.draw_load_force(force_ids,size);
}

bool CalculiXDrawInterface::draw_load_pressure(std::vector<int> pressure_ids,double size)
{
  return ccx_core.draw_load_pressure(pressure_ids,size);
}

bool CalculiXDrawInterface::draw_load_heatflux(std::vector<int> heatflux_ids,double size)
{
  return ccx_core.draw_load_heatflux(heatflux_ids,size);
}

bool CalculiXDrawInterface::draw_load_gravity(std::vector<int> gravity_ids,double size)
{
  return ccx_core.draw_load_gravity(gravity_ids,size);
}

bool CalculiXDrawInterface::draw_load_centrifugal(std::vector<int> centrifugal_ids,double size)
{
  return ccx_core.draw_load_centrifugal(centrifugal_ids,size);
}

bool CalculiXDrawInterface::draw_load_trajectory(std::vector<int> trajectory_ids,double size)
{
  return ccx_core.draw_load_trajectory(trajectory_ids,size);
}

bool CalculiXDrawInterface::draw_load_film(std::vector<int> film_ids,double size)
{
  return ccx_core.draw_load_film(film_ids,size);
}

bool CalculiXDrawInterface::draw_load_radiation(std::vector<int> radiation_ids,double size)
{
  return ccx_core.draw_load_radiation(radiation_ids,size);
}

bool CalculiXDrawInterface::draw_load_surface_traction(std::vector<int> surface_traction_ids,double size)
{
  return ccx_core.draw_load_surface_traction(surface_traction_ids,size);
}

bool CalculiXDrawInterface::draw_load_bodyheatflux(std::vector<int> bodyheatflux_ids,double size)
{
  return ccx_core.draw_load_bodyheatflux(bodyheatflux_ids,size);
}

bool CalculiXDrawInterface::draw_bc_displacement(std::vector<int> displacement_ids,double size)
{
  return ccx_core.draw_bc_displacement(displacement_ids,size);
}

bool CalculiXDrawInterface::draw_bc_temperature(std::vector<int> temperature_ids,double size)
{
  return ccx_core.draw_bc_temperature(temperature_ids,size);
}

bool CalculiXDrawInterface::draw_orientation(std::vector<int> orientation_ids,double size)
{
  return ccx_core.draw_orientation(orientation_ids,size);
}

bool CalculiXDrawInterface::draw_equation(std::vector<int> equation_ids,double size)
{
  return ccx_core.draw_equation(equation_ids,size);
}

bool CalculiXDrawInterface::draw_equationgroup(std::vector<int> equationgroup_ids,double size)
{
  return ccx_core.draw_equationgroup(equationgroup_ids,size);
}

bool CalculiXDrawInterface::draw_loads(double size)
{
  return ccx_core.draw_loads(size);
}

bool CalculiXDrawInterface::draw_bcs(double size)
{
  return ccx_core.draw_bcs(size);
}

bool CalculiXDrawInterface::draw_orientations(double size)
{
  return ccx_core.draw_orientations(size);
}

bool CalculiXDrawInterface::draw_equations(double size)
{
  return ccx_core.draw_equations(size);
}

bool CalculiXDrawInterface::draw_equationgroups(double size)
{
  return ccx_core.draw_equationgroups(size);
}

bool CalculiXDrawInterface::draw_load_forces(double size)
{
  return ccx_core.draw_load_forces(size);
}

bool CalculiXDrawInterface::draw_load_pressures(double size)
{
  return ccx_core.draw_load_pressures(size);
}

bool CalculiXDrawInterface::draw_load_heatfluxes(double size)
{
  return ccx_core.draw_load_heatfluxes(size);
}

bool CalculiXDrawInterface::draw_load_gravities(double size)
{
  return ccx_core.draw_load_gravities(size);
}

bool CalculiXDrawInterface::draw_load_centrifugals(double size)
{
  return ccx_core.draw_load_centrifugals(size);
}

bool CalculiXDrawInterface::draw_load_trajectories(double size)
{
  return ccx_core.draw_load_trajectories(size);
}

bool CalculiXDrawInterface::draw_load_films(double size)
{
  return ccx_core.draw_load_films(size);
}

bool CalculiXDrawInterface::draw_load_radiations(double size)
{
  return ccx_core.draw_load_radiations(size);
}

bool CalculiXDrawInterface::draw_load_surface_tractions(double size)
{
  return ccx_core.draw_load_surface_tractions(size);
}

bool CalculiXDrawInterface::draw_load_bodyheatfluxes(double size)
{
  return ccx_core.draw_load_bodyheatfluxes(size);
}

bool CalculiXDrawInterface::draw_bc_displacements(double size)
{
  return ccx_core.draw_bc_displacements(size);
}

bool CalculiXDrawInterface::draw_bc_temperatures(double size)
{
  return ccx_core.draw_bc_temperatures(size);
}

bool CalculiXDrawInterface::draw_topology(int topology_id,int iteration_id,double threshold)
{
  return ccx_core.draw_topology(topology_id, iteration_id, threshold);
}

bool CalculiXDrawInterface::animate_topology(int topology_id,double threshold,int fps,std::string save_filepath, bool overwrite)
{
  return ccx_core.animate_topology(topology_id, threshold, fps, save_filepath, overwrite);
}

bool CalculiXDrawInterface::topology_animation_active() const
{
  return ccx_core.topology_animation_active();
}

void CalculiXDrawInterface::cancel_topology_animation()
{
  ccx_core.cancel_topology_animation();
}
