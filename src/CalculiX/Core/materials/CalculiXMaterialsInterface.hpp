#ifndef CALCULIXMATERIALSINTERFACE_HPP
#define CALCULIXMATERIALSINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXMaterialsInterface
{
public:
  std::string get_material_name(int material_id); // gets the material name
  std::string get_material_prefix(std::string material_name); // gets a prefix for the material name if needed
  int get_group_property_size(std::string group_property); // gets the size of a material group property
  bool check_material_exists(int material_id); // check if material exists
  std::string get_material_export_data(int topology_id = -1,int iteration_id = -1); // gets the export data from materials core
  bool create_materiallibrary_group(std::string name);
  bool modify_materiallibrary_group(std::string name, std::string data, int mode);
  bool delete_materiallibrary_group(std::string name);
  bool create_materiallibrary_material(std::string name, std::string groupname);
  bool modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data);
  bool delete_materiallibrary_material(std::string name, std::string groupname);
  bool export_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name);
  bool import_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name);
  std::vector<std::vector<std::string>> get_material_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_group_properties(); // gets the data from core to build the item in the material management
  std::vector<std::vector<std::string>> get_materiallibrary_tree_data(); // gets the data from materiallibrary to build the tree in the material management
  std::vector<std::vector<std::string>> get_materiallibrary_material_properties(std::string name, std::string group); // gets the material properties from materiallibrary for the material management
  std::vector<std::vector<double>> get_materiallibrary_material_values(std::string name, std::string group, std::string property); // gets the material values for a property from materiallibrary for the material management
};

#endif // CALCULIXMATERIALSINTERFACE_HPP
