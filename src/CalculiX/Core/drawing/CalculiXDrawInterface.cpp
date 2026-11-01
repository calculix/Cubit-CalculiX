#include "CalculiXDrawInterface.hpp"
#include "CoreDraw.hpp"
#include "CubitMessage.hpp"
#include "CoreDrawBCData.hpp"
#include "CoreDrawLoadData.hpp"
#include "CoreDrawConstraintData.hpp"
#include "CoreDrawOrientationData.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_force(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_force(*ccx_core.loadsforces, *ccx_core.me_iface, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_pressure(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_pressure(*ccx_core.loadspressures, *ccx_core.me_iface, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_heatflux(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_heatflux(*ccx_core.loadsheatfluxes, *ccx_core.me_iface, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_gravity(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_gravity(*ccx_core.loadsgravity, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_centrifugal(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_centrifugal(*ccx_core.loadscentrifugal, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_film(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_film(*ccx_core.loadsfilm, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_radiation(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_radiation(*ccx_core.loadsradiation, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_load_surface_traction(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_surface_traction(*ccx_core.loadssurfacetraction, id);
}

std::vector<std::vector<int>> CalculiXDrawInterface::get_draw_data_for_load_bodyheatflux(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_bodyheatflux(*ccx_core.loadsbodyheatflux, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_bc_displacement(int id) // returns coord(3) and dof
{
  return CoreDrawBCData::get_draw_data_for_bc_displacement(*ccx_core.bcsdisplacements, *ccx_core.me_iface, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_bc_temperature(int id) // returns coord(3) and dof
{
  return CoreDrawBCData::get_draw_data_for_bc_temperature(*ccx_core.bcstemperatures, *ccx_core.me_iface, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_orientation(int id)
{
  return CoreDrawOrientationData::get_draw_data_for_orientation(*ccx_core.orientations, *ccx_core.sections, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_equation(int id)
{
  return CoreDrawConstraintData::get_draw_data_for_equation(*ccx_core.constraints, id);
}

std::vector<std::vector<double>> CalculiXDrawInterface::get_draw_data_for_equationgroup(int id)
{
  return CoreDrawConstraintData::get_draw_data_for_equationgroup(*ccx_core.constraints, id);
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

bool CalculiXDrawInterface::draw_topology(int topology_id, int iteration_id, double threshold)
{
  if (!ccx_core.draw)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }

  return ccx_core.draw->draw_topology(topology_id, iteration_id, threshold);
}

bool CalculiXDrawInterface::animate_topology(int topology_id, double threshold, int fps, std::string save_filepath, bool overwrite)
{
  if (!ccx_core.draw)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }

  return ccx_core.draw->animate_topology(topology_id, threshold, fps, save_filepath, overwrite);
}

bool CalculiXDrawInterface::topology_animation_active() const
{
  return ccx_core.draw && ccx_core.draw->topology_animation_active();
}

void CalculiXDrawInterface::cancel_topology_animation()
{
  if (ccx_core.draw)
  {
    ccx_core.draw->cancel_topology_animation();
  }
}
