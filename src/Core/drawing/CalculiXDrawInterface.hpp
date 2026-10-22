#ifndef CALCULIXDRAWINTERFACE_HPP
#define CALCULIXDRAWINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXDrawInterface
{
public:
  std::vector<std::vector<double>> get_draw_data_for_load_force(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_pressure(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_heatflux(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_gravity(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_centrifugal(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_film(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_radiation(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_surface_traction(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<int>> get_draw_data_for_load_bodyheatflux(int id); // returns block ids draw_data[][0] element ids draw_data[][1]
  std::vector<std::vector<double>> get_draw_data_for_bc_displacement(int id); // returns coord(3) and dof
  std::vector<std::vector<double>> get_draw_data_for_bc_temperature(int id); // returns coord(3) and dof
  std::vector<std::vector<double>> get_draw_data_for_orientation(int id); // returns pairs of 4 for {system_type,local_axis_angle}, coord(3) of section center, a_coord(3) ,b_coord(3)
  std::vector<std::vector<double>> get_draw_data_for_equation(int id); // returns coord(3) and dof for each variable
  std::vector<std::vector<double>> get_draw_data_for_equationgroup(int id); // returns coord(3) and dof for each variable
  bool draw_all(double size); // draw all loads,bcs,orientations,equations
  bool draw_load_force(std::vector<int> force_ids,double size); // draw load force
  bool draw_load_pressure(std::vector<int> pressure_ids,double size); // draw load pressure
  bool draw_load_heatflux(std::vector<int> heatflux_ids,double size); // draw load heatflux
  bool draw_load_gravity(std::vector<int> gravity_ids,double size); // draw load gravity
  bool draw_load_centrifugal(std::vector<int> centrifugal_ids,double size); // draw load centrifugal
  bool draw_load_trajectory(std::vector<int> trajectory_ids,double size); // draw load trajectory
  bool draw_load_film(std::vector<int> film_ids,double size); // draw load film
  bool draw_load_radiation(std::vector<int> radiation_ids,double size); // draw load radiation
  bool draw_load_surface_traction(std::vector<int> surface_traction_ids,double size); // draw load surface traction
  bool draw_load_bodyheatflux(std::vector<int> bodyheatflux_ids,double size); // draw load surface traction
  bool draw_bc_displacement(std::vector<int> displacement_ids,double size); // draw bc displacement
  bool draw_bc_temperature(std::vector<int> temperature_ids,double size); // draw bc temperature
  bool draw_orientation(std::vector<int> orientation_ids,double size); // draw orientation
  bool draw_equation(std::vector<int> equation_ids,double size); // draw equation
  bool draw_equationgroup(std::vector<int> equationgroup_ids,double size); // draw equation
  bool draw_loads(double size); // draw all loads
  bool draw_bcs(double size); // draw all bcs
  bool draw_orientations(double size); // draw all orientations
  bool draw_equations(double size); // draw all equations
  bool draw_equationgroups(double size); // draw all equationgroups
  bool draw_load_forces(double size); //draw all forces
  bool draw_load_pressures(double size); //draw all pressures
  bool draw_load_heatfluxes(double size); //draw all heatfluxes
  bool draw_load_gravities(double size); //draw all gravities
  bool draw_load_centrifugals(double size); //draw all centrifugals
  bool draw_load_trajectories(double size); //draw all trajectories
  bool draw_load_films(double size); //draw all films
  bool draw_load_radiations(double size); //draw all radiations
  bool draw_load_surface_tractions(double size); //draw all surface tractions
  bool draw_load_bodyheatfluxes(double size); //draw all bodyheatflux
  bool draw_bc_displacements(double size); //draw all displacements
  bool draw_bc_temperatures(double size); //draw all temperatures
  bool draw_topology(int topology_id,int iteration_id = -1,double threshold = 0.5); // draw topology iteration
  bool animate_topology(int topology_id,double threshold = 0.5,int fps = 5,std::string save_filepath = "", bool overwrite = false);
};

#endif // CALCULIXDRAWINTERFACE_HPP
