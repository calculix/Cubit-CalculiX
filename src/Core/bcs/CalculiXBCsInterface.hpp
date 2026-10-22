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
  bool modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a displacement
  bool modify_bcstemperatures(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a temperature
  bool hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // adds bcs to bcs_data
  bool hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // removes bcs from bcs_data
  std::vector<std::vector<std::string>> get_bcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_bcstemperatures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_hbcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_hbcstemperatures_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXBCSINTERFACE_HPP
