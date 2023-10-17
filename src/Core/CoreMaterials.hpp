#ifndef COREMATERIALS_HPP
#define COREMATERIALS_HPP

#include <vector>
#include <string>

class MaterialInterface;
class CalculiXCoreInterface;

class CoreMaterials
{

public:
  CoreMaterials();
  ~CoreMaterials();

  std::vector<std::vector<std::string>> material_cards; // used to store the material cards and its property prefix, maybe comes in handy later
  // materials_data[0][0] material_card
  // materials_data[0][1] material card property prefix
  std::vector<std::vector<std::string>> group_properties;
  std::vector<std::string> group_properties_description;
  // group_properties[0][0] property name
  // group_properties[0][1] property name type  1=scalar, 2=vector, 3 = matrix, 4 = tabular
  // group_properties[0][2] type column size for table property

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the materials // just a placeholder right now
  bool reset(); // delete all data and initialize afterwards // just a placeholder right now
  bool check_initialized(); // check if object is initialized
  std::vector<std::string> get_group_list(); // get a list of CalculiX Material Groups
  std::string get_material_export(); // get CalculiX Material exports
  std::string get_material_cards_export(std::string material_name, std::string group_name); // get card exports for a material
  bool check_material_exists(int material_id); // check if material exists
  std::string get_material_name(int material_id); // gets material name

  MaterialInterface *mat_iface;
  CalculiXCoreInterface *ccx_iface;
};

#endif // COREMATERIALS_HPP
