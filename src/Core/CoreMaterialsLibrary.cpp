#include "CoreMaterialsLibrary.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"


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