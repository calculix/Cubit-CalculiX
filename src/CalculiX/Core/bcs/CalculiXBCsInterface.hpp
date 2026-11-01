#ifndef CALCULIXBCSINTERFACE_HPP
#define CALCULIXBCSINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXBCsInterface
{
public:
  std::vector<int> get_bcsdisplacements_ids(); // get all bc displacement ids
  std::vector<int> get_bcstemperatures_ids(); // get all bc temperature ids
  std::string get_hbc_export_data(); // gets the export data from core
  bool create_bcsdisplacements(std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3);
  bool delete_bcsdisplacements(int displacement_id);
  bool modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3, std::vector<int> options_marker); // modify a displacement
  bool create_bcstemperatures(std::vector<std::string> options, std::vector<double> options2);
  bool modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_bcstemperatures(int temperature_id);
  bool hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // adds bcs to bcs_data
  bool hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // removes bcs from bcs_data
  std::vector<std::vector<std::string>> get_bcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_bcstemperatures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_hbcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_hbcstemperatures_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXBCSINTERFACE_HPP
