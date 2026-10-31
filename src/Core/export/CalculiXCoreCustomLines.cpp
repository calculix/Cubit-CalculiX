#include "Core/CalculiXCore.hpp"
#include "CoreCustomLines.hpp"

std::vector<int> CalculiXCore::get_customline_ids()
{
  return customlines->get_customline_ids();
}

bool CalculiXCore::create_customline(std::vector<std::string> options)
{
  return customlines->create_customline(options);
}

bool CalculiXCore::modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return customlines->modify_customline(customline_id, options, options_marker);
}

bool CalculiXCore::delete_customline(int customline_id)
{
  return customlines->delete_customline(customline_id);
}

std::vector<std::string> CalculiXCore::get_customline_data(std::string position, std::string keyword, int keyword_id)
{
  return customlines->get_customline_data(position, keyword, keyword_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_customlines_tree_data()
{
  return customlines->get_tree_data();
}
