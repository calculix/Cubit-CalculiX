#ifndef CALCULIXSECTIONSINTERFACE_HPP
#define CALCULIXSECTIONSINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXSectionsInterface
{
public:
  std::string get_orientation_name(int orientation_id); // gets the orientation name
  std::vector<int> get_orientations_ids(); // get all orientation ids
  bool check_orientation_exists(int orientation_id); // check if orientation exists
  std::string get_section_export_data(int topology_id = -1,int iteration_id = -1); // gets the export data from sections core
  std::string get_orientation_export_data(); // gets the export data from orientations core
  bool create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options); // adds a new section
  bool modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a section
  bool delete_section(int section_id); // delete section
  bool create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new orientation
  bool modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a orientation
  bool delete_orientation(int orientation_id); // delete orientation
  std::vector<std::vector<std::string>> get_sections_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_orientations_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXSECTIONSINTERFACE_HPP
