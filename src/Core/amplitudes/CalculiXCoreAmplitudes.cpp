#include "Core/CalculiXCore.hpp"
#include "CoreAmplitudes.hpp"

std::string CalculiXCore::get_amplitude_name(int amplitude_id)
{
  return amplitudes->get_amplitude_name(amplitude_id);
}

bool CalculiXCore::check_amplitude_exists(int amplitude_id)
{
  return amplitudes->check_amplitude_exists(amplitude_id);
}

bool CalculiXCore::create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return amplitudes->create_amplitude(options, options2);
}

bool CalculiXCore::modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return amplitudes->modify_amplitude(amplitude_id, options, options_marker, options2);
}

bool CalculiXCore::delete_amplitude(int amplitude_id)
{
  return amplitudes->delete_amplitude(amplitude_id);
}

bool CalculiXCore::delete_amplitudes(std::vector<int> amplitude_ids)
{
  return amplitudes->delete_amplitudes(amplitude_ids);
}

std::string CalculiXCore::get_amplitude_export_data()
{
  return amplitudes->get_amplitude_export();
}

std::vector<std::vector<std::string>> CalculiXCore::get_amplitudes_tree_data()
{
  return amplitudes->get_tree_data();
}
