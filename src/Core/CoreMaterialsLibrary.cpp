#include "CoreMaterialsLibrary.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
#include "loadUserOptions.hpp"
#include "HDF5Tool.hpp"


CoreMaterialsLibrary::CoreMaterialsLibrary()
{}

CoreMaterialsLibrary::~CoreMaterialsLibrary()
{}

bool CoreMaterialsLibrary::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

    is_initialized = true;
    this->load_library();

    return true;
  }
}

bool CoreMaterialsLibrary::update()
{ 
  return true;
}

bool CoreMaterialsLibrary::reset()
{
  //init();
  return true;
}

bool CoreMaterialsLibrary::check_initialized()
{
  return is_initialized;
}

bool CoreMaterialsLibrary::load_library()
{
  std::string log = "Loading Material Library File " + ccx_uo.mPathMaterialLibrary.toStdString() + "\n";
  PRINT_INFO("%s", log.c_str());

  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  
  return true;
}


bool CoreMaterialsLibrary::create_group(std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  if (hdf5Tool.nameExists(groupname))
  {
    log = "Can't create new group in the Material Library.\n" + groupname + " already exists!\n";
    PRINT_INFO("%s", log.c_str());

    return false;
  }

  hdf5Tool.createGroup(groupname);
  log = "Create new group " + groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool CoreMaterialsLibrary::rename_group(std::string groupname, std::string new_groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  log = "Rename group " + groupname + " to " + new_groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}  

bool CoreMaterialsLibrary::delete_group(std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  log = "Delete group " + groupname + " from the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}  

bool CoreMaterialsLibrary::check_group(std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());

  if (hdf5Tool.nameExists(groupname))
  {
    return true;
  }

  return false;
}
