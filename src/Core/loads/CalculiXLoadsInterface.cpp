#include "CalculiXLoadsInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<int> CalculiXLoadsInterface::get_loadsforces_ids()
{
  return ccx_core.get_loadsforces_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadspressures_ids()
{
  return ccx_core.get_loadspressures_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsheatfluxes_ids()
{
  return ccx_core.get_loadsheatfluxes_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsgravity_ids()
{
  return ccx_core.get_loadsgravity_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadscentrifugal_ids()
{
  return ccx_core.get_loadscentrifugal_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadstrajectory_ids()
{
  return ccx_core.get_loadstrajectory_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsfilm_ids()
{
  return ccx_core.get_loadsfilm_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsradiation_ids()
{
  return ccx_core.get_loadsradiation_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadssurfacetraction_ids()
{
  return ccx_core.get_loadssurfacetraction_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsbodyheatflux_ids()
{
  return ccx_core.get_loadsbodyheatflux_ids();
}

bool CalculiXLoadsInterface::delete_loadsforces(int force_id)
{
  return ccx_core.delete_loadsforces(force_id);
}

bool CalculiXLoadsInterface::create_loadsforces(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.create_loadsforces(options, options2);
}

bool CalculiXLoadsInterface::modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsforces(force_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadspressures(pressure_id,options,options_marker);
}

bool CalculiXLoadsInterface::modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsheatfluxes(heatflux_id,options,options_marker);
}

bool CalculiXLoadsInterface::create_loadsgravity(std::vector<std::string> options)
{
  return ccx_core.create_loadsgravity(options);
}

bool CalculiXLoadsInterface::modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsgravity(gravity_id,options,options_marker);
}

bool CalculiXLoadsInterface::delete_loadsgravity(int gravity_id)
{
  return ccx_core.delete_loadsgravity(gravity_id);
}

bool CalculiXLoadsInterface::create_loadscentrifugal(std::vector<std::string> options)
{
  return ccx_core.create_loadscentrifugal(options);
}

bool CalculiXLoadsInterface::modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadscentrifugal(centrifugal_id,options,options_marker);
}

bool CalculiXLoadsInterface::delete_loadscentrifugal(int centrifugal_id)
{
  return ccx_core.delete_loadscentrifugal(centrifugal_id);
}

bool CalculiXLoadsInterface::create_loadstrajectory(std::string load_type, std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return ccx_core.create_loadstrajectory(load_type, options, options2, options3);
}

bool CalculiXLoadsInterface::modify_loadstrajectory(int trajectory_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return ccx_core.modify_loadstrajectory(trajectory_id,options,options_marker, options2, options3);
}

bool CalculiXLoadsInterface::delete_loadstrajectory(int trajectory_id)
{
  return ccx_core.delete_loadstrajectory(trajectory_id);
}

std::string CalculiXLoadsInterface::loadstrajectory_get_load_type(int trajectory_id)
{
  return ccx_core.loadstrajectory_get_load_type(trajectory_id);
}

int CalculiXLoadsInterface::loadstrajectory_get_subload_id(int trajectory_id)
{
  return ccx_core.loadstrajectory_get_subload_id(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_heatflux_get_node_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_node_ids(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_heatflux_get_edge_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_edge_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_hit_coordinates(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_hit_coordinates(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_face_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_face_ids(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_draw_face_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_draw_face_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_times(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_times(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_radius(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_radius(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_magnitude(int trajectory_id)
{
  return ccx_core.loadstrajectory_heatflux_get_magnitude(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_node_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_node_ids(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_edge_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_edge_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_element_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_element_ids(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_times(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_times(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_radius(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_radius(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_depth(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_depth(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_magnitude(int trajectory_id)
{
  return ccx_core.loadstrajectory_bodyheatfluxsphere_get_magnitude(trajectory_id);
}

bool CalculiXLoadsInterface::create_loadsfilm(std::vector<std::string> options)
{
  return ccx_core.create_loadsfilm(options);
}

bool CalculiXLoadsInterface::modify_loadsfilm(int film_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsfilm(film_id, options, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsfilm(int film_id)
{
  return ccx_core.delete_loadsfilm(film_id);
}

bool CalculiXLoadsInterface::create_loadsradiation(std::vector<std::string> options)
{
  return ccx_core.create_loadsradiation(options);
}

bool CalculiXLoadsInterface::modify_loadsradiation(int radiation_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsradiation(radiation_id, options, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsradiation(int radiation_id)
{
  return ccx_core.delete_loadsradiation(radiation_id);
}

bool CalculiXLoadsInterface::create_loadssurfacetraction(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.create_loadssurfacetraction(options, options2);
}

bool CalculiXLoadsInterface::modify_loadssurfacetraction(int surfacetraction_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.modify_loadssurfacetraction(surfacetraction_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::delete_loadssurfacetraction(int surfacetraction_id)
{
  return ccx_core.delete_loadssurfacetraction(surfacetraction_id);
}

bool CalculiXLoadsInterface::create_loadsbodyheatflux(std::vector<std::string> options, std::vector<int> options2)
{
  return ccx_core.create_loadsbodyheatflux(options, options2);
}

bool CalculiXLoadsInterface::modify_loadsbodyheatflux(int bodyheatflux_id, std::vector<std::string> options, std::vector<int> options2, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsbodyheatflux(bodyheatflux_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsbodyheatflux(int bodyheatflux_id)
{
  return ccx_core.delete_loadsbodyheatflux(bodyheatflux_id);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsforces_tree_data()
{ 
  return ccx_core.get_loadsforces_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadspressures_tree_data()
{ 
  return ccx_core.get_loadspressures_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsheatfluxes_tree_data()
{ 
  return ccx_core.get_loadsheatfluxes_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsgravity_tree_data()
{ 
  return ccx_core.get_loadsgravity_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadscentrifugal_tree_data()
{ 
  return ccx_core.get_loadscentrifugal_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadstrajectory_tree_data()
{ 
  return ccx_core.get_loadstrajectory_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsfilm_tree_data()
{ 
  return ccx_core.get_loadsfilm_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsradiation_tree_data()
{ 
  return ccx_core.get_loadsradiation_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadssurfacetraction_tree_data()
{ 
  return ccx_core.get_loadssurfacetraction_tree_data();
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsbodyheatflux_tree_data()
{ 
  return ccx_core.get_loadsbodyheatflux_tree_data();
}
