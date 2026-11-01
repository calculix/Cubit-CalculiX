#include "CoreLoadRemoval.hpp"
#include "CalculiXLoadsInterface.hpp"
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
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<int> CalculiXLoadsInterface::get_loadsforces_ids()
{
  return ccx_core.loadsforces->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadspressures_ids()
{
  return ccx_core.loadspressures->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsheatfluxes_ids()
{
  return ccx_core.loadsheatfluxes->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsgravity_ids()
{
  return ccx_core.loadsgravity->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadscentrifugal_ids()
{
  return ccx_core.loadscentrifugal->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadstrajectory_ids()
{
  return ccx_core.loadstrajectory->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsfilm_ids()
{
  return ccx_core.loadsfilm->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsradiation_ids()
{
  return ccx_core.loadsradiation->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadssurfacetraction_ids()
{
  return ccx_core.loadssurfacetraction->get_load_ids();
}

std::vector<int> CalculiXLoadsInterface::get_loadsbodyheatflux_ids()
{
  return ccx_core.loadsbodyheatflux->get_load_ids();
}

bool CalculiXLoadsInterface::delete_loadsforces(int force_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadsforces, *ccx_core.steps, force_id);
}

bool CalculiXLoadsInterface::create_loadsforces(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.loadsforces->create_load(options, options2);
}

bool CalculiXLoadsInterface::modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.loadsforces->modify_load(force_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::delete_loadspressures(int pressure_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadspressures, *ccx_core.steps, pressure_id);
}

bool CalculiXLoadsInterface::create_loadspressures(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.loadspressures->create_load(options, options2);
}

bool CalculiXLoadsInterface::modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.loadspressures->modify_load(pressure_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::create_loadsheatfluxes(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.loadsheatfluxes->create_load(options, options2);
}

bool CalculiXLoadsInterface::modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.loadsheatfluxes->modify_load(heatflux_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsheatfluxes(int heatflux_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadsheatfluxes, *ccx_core.steps, heatflux_id);
}

bool CalculiXLoadsInterface::create_loadsgravity(std::vector<std::string> options)
{
  return ccx_core.loadsgravity->create_load(options);
}

bool CalculiXLoadsInterface::modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.loadsgravity->modify_load(gravity_id, options, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsgravity(int gravity_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadsgravity, *ccx_core.steps, gravity_id);
}

bool CalculiXLoadsInterface::create_loadscentrifugal(std::vector<std::string> options)
{
  return ccx_core.loadscentrifugal->create_load(options);
}

bool CalculiXLoadsInterface::modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.loadscentrifugal->modify_load(centrifugal_id, options, options_marker);
}

bool CalculiXLoadsInterface::delete_loadscentrifugal(int centrifugal_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadscentrifugal, *ccx_core.steps, centrifugal_id);
}

bool CalculiXLoadsInterface::create_loadstrajectory(std::string load_type, std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return ccx_core.loadstrajectory->create_load(load_type, options, options2, options3);
}

bool CalculiXLoadsInterface::modify_loadstrajectory(int trajectory_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return ccx_core.loadstrajectory->modify_load(trajectory_id, options, options_marker, options2, options3);
}

bool CalculiXLoadsInterface::delete_loadstrajectory(int trajectory_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadstrajectory, *ccx_core.steps, trajectory_id);
}

std::string CalculiXLoadsInterface::loadstrajectory_get_load_type(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_load_type(trajectory_id);
}

int CalculiXLoadsInterface::loadstrajectory_get_subload_id(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_subload_id(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_heatflux_get_node_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_node_ids(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_heatflux_get_edge_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_edge_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_hit_coordinates(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_hit_coordinates(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_face_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_face_ids(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_draw_face_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_draw_face_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_times(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_times(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_radius(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_radius(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_heatflux_get_magnitude(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_heatflux_magnitude(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_node_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_node_ids(trajectory_id);
}

std::vector<int> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_edge_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_edge_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_hit_coordinates(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_element_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_element_ids(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_draw_element_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_times(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_times(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_radius(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_radius(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_depth(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_depth(trajectory_id);
}

std::vector<std::vector<double>> CalculiXLoadsInterface::loadstrajectory_bodyheatfluxsphere_get_magnitude(int trajectory_id)
{
  return ccx_core.loadstrajectory->get_bodyheatfluxsphere_magnitude(trajectory_id);
}

bool CalculiXLoadsInterface::create_loadsfilm(std::vector<std::string> options)
{
  return ccx_core.loadsfilm->create_load(options);
}

bool CalculiXLoadsInterface::modify_loadsfilm(int film_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.loadsfilm->modify_load(film_id, options, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsfilm(int film_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadsfilm, *ccx_core.steps, film_id);
}

bool CalculiXLoadsInterface::create_loadsradiation(std::vector<std::string> options)
{
  return ccx_core.loadsradiation->create_load(options);
}

bool CalculiXLoadsInterface::modify_loadsradiation(int radiation_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.loadsradiation->modify_load(radiation_id, options, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsradiation(int radiation_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadsradiation, *ccx_core.steps, radiation_id);
}

bool CalculiXLoadsInterface::create_loadssurfacetraction(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.loadssurfacetraction->create_load(options, options2);
}

bool CalculiXLoadsInterface::modify_loadssurfacetraction(int surfacetraction_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.loadssurfacetraction->modify_load(surfacetraction_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::delete_loadssurfacetraction(int surfacetraction_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadssurfacetraction, *ccx_core.steps, surfacetraction_id);
}

bool CalculiXLoadsInterface::create_loadsbodyheatflux(std::vector<std::string> options, std::vector<int> options2)
{
  return ccx_core.loadsbodyheatflux->create_load(options, options2);
}

bool CalculiXLoadsInterface::modify_loadsbodyheatflux(int bodyheatflux_id, std::vector<std::string> options, std::vector<int> options2, std::vector<int> options_marker)
{
  return ccx_core.loadsbodyheatflux->modify_load(bodyheatflux_id, options, options2, options_marker);
}

bool CalculiXLoadsInterface::delete_loadsbodyheatflux(int bodyheatflux_id)
{
  return CoreLoadRemoval::delete_load(*ccx_core.loadsbodyheatflux, *ccx_core.steps, bodyheatflux_id);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsforces_tree_data()
{ 
  return CoreLoadsTreeData::get_loadsforces_tree_data(*ccx_core.loadsforces);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadspressures_tree_data()
{ 
  return CoreLoadsTreeData::get_loadspressures_tree_data(*ccx_core.loadspressures);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsheatfluxes_tree_data()
{ 
  return CoreLoadsTreeData::get_loadsheatfluxes_tree_data(*ccx_core.loadsheatfluxes);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsgravity_tree_data()
{ 
  return CoreLoadsTreeData::get_loadsgravity_tree_data(*ccx_core.loadsgravity);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadscentrifugal_tree_data()
{ 
  return CoreLoadsTreeData::get_loadscentrifugal_tree_data(*ccx_core.loadscentrifugal);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadstrajectory_tree_data()
{ 
  return CoreLoadsTreeData::get_loadstrajectory_tree_data(*ccx_core.loadstrajectory);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsfilm_tree_data()
{ 
  return CoreLoadsTreeData::get_loadsfilm_tree_data(*ccx_core.loadsfilm);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsradiation_tree_data()
{ 
  return CoreLoadsTreeData::get_loadsradiation_tree_data(*ccx_core.loadsradiation);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadssurfacetraction_tree_data()
{ 
  return CoreLoadsTreeData::get_loadssurfacetraction_tree_data(*ccx_core.loadssurfacetraction);
}

std::vector<std::vector<std::string>> CalculiXLoadsInterface::get_loadsbodyheatflux_tree_data()
{ 
  return CoreLoadsTreeData::get_loadsbodyheatflux_tree_data(*ccx_core.loadsbodyheatflux);
}
