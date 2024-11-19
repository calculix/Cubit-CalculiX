#ifndef COREMATERIALSLIBRARY_HPP
#define COREMATERIALSLIBRARY_HPP

/*!
 * \brief This object is used to load and save data from/into the hdf5 library. The materials can be loaded into cubit and saved from cubit into the library.
 */

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreMaterialsLibrary
{

public:
  CoreMaterialsLibrary();
  ~CoreMaterialsLibrary();

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the materials library
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool load_library(); // load the library, will create a empty library file if no file is found
  bool create_group(std::string groupname); // create a new group
  bool rename_group(std::string groupname, std::string new_groupname); // renames a group
  bool delete_group(std::string groupname); // deletes the group
  bool create_material(std::string name, std::string groupname); // create a new material in a group
  bool rename_material(std::string name, std::string new_name, std::string groupname); // renames a material in a group
  bool delete_material(std::string name, std::string groupname); // deletes the material from a group

  bool check_group(std::string groupname); // check if the group exists
  bool check_material(std::string name, std::string groupname); // check if the material exists in the group
  
  //bool get_groups(); // gets the groups from the library
  //bool get_materials(); // gets the materials from the library

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREMATERIALS_HPP
