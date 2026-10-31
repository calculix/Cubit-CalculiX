#include "Core/CalculiXCore.hpp"
#include "CoreFieldOutputs.hpp"
#include "CoreOutputsTreeData.hpp"

bool CalculiXCore::check_fieldoutput_exists(int fieldoutput_id)
{
  return fieldoutputs->check_fieldoutput_exists(fieldoutput_id);
}

bool CalculiXCore::create_fieldoutput(std::vector<std::string> options)
{
  return fieldoutputs->create_output(options);
}

bool CalculiXCore::modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return fieldoutputs->modify_output(output_id, modify_type, options, options_marker);
}

bool CalculiXCore::delete_fieldoutput(int output_id)
{
  return fieldoutputs->delete_output(output_id);
}

std::vector<std::string> CalculiXCore::get_fieldoutput_node_keys()
{
  return fieldoutputs->node_keys;
}

std::vector<std::string> CalculiXCore::get_fieldoutput_element_keys()
{
  return fieldoutputs->element_keys;
}

std::vector<std::string> CalculiXCore::get_fieldoutput_contact_keys()
{
  return fieldoutputs->contact_keys;
}

std::vector<std::vector<std::string>> CalculiXCore::get_fieldoutputs_tree_data()
{
  return CoreOutputsTreeData::get_fieldoutputs_tree_data(*fieldoutputs);
}
