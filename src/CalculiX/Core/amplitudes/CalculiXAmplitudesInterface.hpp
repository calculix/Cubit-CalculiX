#ifndef CALCULIXAMPLITUDESINTERFACE_HPP
#define CALCULIXAMPLITUDESINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXAmplitudesInterface
{
public:
  std::string get_amplitude_name(int amplitude_id); // gets the amplitude name
  std::string get_amplitude_export_data(); // gets the export data from amplitudes core
  bool create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new amplitude
  bool modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a amplitude
  bool delete_amplitude(int amplitude_id); // delete amplitude
  bool delete_amplitudes(std::vector<int> amplitude_ids); // deletes amplitudes
  std::vector<std::vector<std::string>> get_amplitudes_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXAMPLITUDESINTERFACE_HPP
