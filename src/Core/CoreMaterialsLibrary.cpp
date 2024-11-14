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
  //HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString().c_str());

  std::string log = "Loading Library\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}