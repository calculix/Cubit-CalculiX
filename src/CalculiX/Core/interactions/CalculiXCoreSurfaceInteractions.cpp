#include "Core/CalculiXCore.hpp"
#include "CoreSurfaceInteractions.hpp"
#include "CoreInteractionsTreeData.hpp"

std::string CalculiXCore::get_surfaceinteraction_name(int surfaceinteraction_id)
{
  return surfaceinteractions->get_surfaceinteraction_name(surfaceinteraction_id);
}

bool CalculiXCore::check_surfaceinteraction_exists(int surfaceinteraction_id)
{
  return surfaceinteractions->check_surfaceinteraction_exists(surfaceinteraction_id);
}

bool CalculiXCore::create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return surfaceinteractions->create_surfaceinteraction(surfacebehavior_type, options, options2);
}

bool CalculiXCore::modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return surfaceinteractions->modify_surfaceinteraction(modify_type, surfaceinteraction_id, options, options_marker, options2);
}

bool CalculiXCore::delete_surfaceinteraction(int surfaceinteraction_id)
{
  return surfaceinteractions->delete_surfaceinteraction(surfaceinteraction_id);
}

std::string CalculiXCore::get_surfaceinteraction_export_data()
{
  return surfaceinteractions->get_surfaceinteraction_export();
}

std::vector<std::vector<std::string>> CalculiXCore::get_surfaceinteractions_tree_data()
{
  return CoreInteractionsTreeData::get_surfaceinteractions_tree_data(*surfaceinteractions);
}
