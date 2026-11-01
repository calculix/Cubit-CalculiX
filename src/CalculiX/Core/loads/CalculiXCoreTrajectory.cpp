#include "Core/CalculiXCore.hpp"
#include "CoreLoadsTrajectory.hpp"
#include "CoreLoadRemoval.hpp"
#include "CoreLoadsTreeData.hpp"

std::vector<int> CalculiXCore::get_loadstrajectory_ids()
{
  return loadstrajectory->get_load_ids();
}

bool CalculiXCore::create_loadstrajectory(std::string load_type, std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return loadstrajectory->create_load(load_type, options, options2, options3);
}

bool CalculiXCore::modify_loadstrajectory(int trajectory_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return loadstrajectory->modify_load(trajectory_id, options, options_marker, options2, options3);
}

bool CalculiXCore::delete_loadstrajectory(int trajectory_id)
{
  return CoreLoadRemoval::delete_load(*loadstrajectory, *steps, trajectory_id);
}

std::string CalculiXCore::loadstrajectory_get_load_type(int trajectory_id)
{
  return loadstrajectory->get_load_type(trajectory_id);
}

int CalculiXCore::loadstrajectory_get_subload_id(int trajectory_id)
{
  return loadstrajectory->get_subload_id(trajectory_id);
}

bool CalculiXCore::prepare_export()
{
  return loadstrajectory->prepare_export_if_needed();
}

bool CalculiXCore::clean_export()
{
  return loadstrajectory->clean_export_if_needed();
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadstrajectory_tree_data()
{
  return CoreLoadsTreeData::get_loadstrajectory_tree_data(*loadstrajectory);
}

std::vector<int> CalculiXCore::loadstrajectory_heatflux_get_node_ids(int trajectory_id)
{
  return loadstrajectory->get_heatflux_node_ids(trajectory_id);
}

std::vector<int> CalculiXCore::loadstrajectory_heatflux_get_edge_ids(int trajectory_id)
{
  return loadstrajectory->get_heatflux_edge_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_hit_coordinates(int trajectory_id)
{
  return loadstrajectory->get_heatflux_hit_coordinates(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_heatflux_get_face_ids(int trajectory_id)
{
  return loadstrajectory->get_heatflux_face_ids(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_heatflux_get_draw_face_ids(int trajectory_id)
{
  return loadstrajectory->get_heatflux_draw_face_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_times(int trajectory_id)
{
  return loadstrajectory->get_heatflux_times(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_radius(int trajectory_id)
{
  return loadstrajectory->get_heatflux_radius(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_magnitude(int trajectory_id)
{
  return loadstrajectory->get_heatflux_magnitude(trajectory_id);
}

std::vector<int> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_node_ids(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_node_ids(trajectory_id);
}

std::vector<int> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_edge_ids(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_edge_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_hit_coordinates(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_element_ids(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_element_ids(trajectory_id);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_draw_element_ids(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_times(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_times(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_radius(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_radius(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_depth(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_depth(trajectory_id);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_magnitude(int trajectory_id)
{
  return loadstrajectory->get_bodyheatfluxsphere_magnitude(trajectory_id);
}
