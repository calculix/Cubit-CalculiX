#include "Core/CalculiXCore.hpp"
#include "CoreHistoryOutputs.hpp"
#include "CoreOutputsTreeData.hpp"

bool CalculiXCore::check_historyoutput_exists(int historyoutput_id)
{
  return historyoutputs->check_historyoutput_exists(historyoutput_id);
}

bool CalculiXCore::create_historyoutput(std::vector<std::string> options)
{
  return historyoutputs->create_output(options);
}

bool CalculiXCore::modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return historyoutputs->modify_output(output_id, modify_type, options, options_marker);
}

bool CalculiXCore::delete_historyoutput(int output_id)
{
  return historyoutputs->delete_output(output_id);
}

std::vector<std::string> CalculiXCore::get_historyoutput_node_keys()
{
  return historyoutputs->node_keys;
}

std::vector<std::string> CalculiXCore::get_historyoutput_element_keys()
{
  return historyoutputs->element_keys;
}

std::vector<std::string> CalculiXCore::get_historyoutput_contact_keys()
{
  return historyoutputs->contact_keys;
}

std::vector<std::string> CalculiXCore::get_historyoutput_section_keys()
{
  return historyoutputs->section_keys;
}

std::vector<std::vector<std::string>> CalculiXCore::get_historyoutputs_tree_data()
{
  return CoreOutputsTreeData::get_historyoutputs_tree_data(*historyoutputs);
}
