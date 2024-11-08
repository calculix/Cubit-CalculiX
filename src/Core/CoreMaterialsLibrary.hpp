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

  //

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the materials library
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  //bool get_groups(); // gets the groups from the library
  //bool get_materials(); // gets the materials from the library

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREMATERIALS_HPP
