#include "CalculiXExportInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<int> CalculiXExportInterface::get_customline_ids()
{
  return ccx_core.get_customline_ids();
}

bool CalculiXExportInterface::prepare_export()
{
  return ccx_core.prepare_export();
}

bool CalculiXExportInterface::clean_export()
{
  return ccx_core.clean_export();
}

bool CalculiXExportInterface::create_customline(std::vector<std::string> options)
{
  return ccx_core.create_customline(options);
}

bool CalculiXExportInterface::modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_customline(customline_id, options, options_marker);
}

bool CalculiXExportInterface::delete_customline(int customline_id)
{
  return ccx_core.delete_customline(customline_id);
}

std::vector<std::string> CalculiXExportInterface::get_customline_data(std::string position,std::string keyword,int keyword_id)
{
  return ccx_core.get_customline_data(position,keyword,keyword_id);
}

std::vector<std::vector<std::string>> CalculiXExportInterface::get_customlines_tree_data()
{ 
  return ccx_core.get_customlines_tree_data();
}
