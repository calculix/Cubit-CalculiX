#include "CalculiXAmplitudesInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::string CalculiXAmplitudesInterface::get_amplitude_name(int amplitude_id)
{
  return ccx_core.get_amplitude_name(amplitude_id);
}

std::string CalculiXAmplitudesInterface::get_amplitude_export_data() // gets the export data from amplitudes core
{
  return ccx_core.get_amplitude_export_data();
}

bool CalculiXAmplitudesInterface::create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.create_amplitude(options, options2);
}

bool CalculiXAmplitudesInterface::modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.modify_amplitude(amplitude_id,options,options_marker, options2);
}

bool CalculiXAmplitudesInterface::delete_amplitude(int amplitude_id)
{
  return ccx_core.delete_amplitude(amplitude_id);
}

bool CalculiXAmplitudesInterface::delete_amplitudes(std::vector<int> amplitude_ids)
{
  return ccx_core.delete_amplitudes(amplitude_ids);
}

std::vector<std::vector<std::string>> CalculiXAmplitudesInterface::get_amplitudes_tree_data()
{ 
  return ccx_core.get_amplitudes_tree_data();
}
