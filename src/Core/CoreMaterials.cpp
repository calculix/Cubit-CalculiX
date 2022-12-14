#include "CoreMaterials.hpp"
#include "CubitInterface.hpp"
#include "MaterialInterface.hpp"



CoreMaterials::CoreMaterials()
{}

CoreMaterials::~CoreMaterials()
{}

bool CoreMaterials::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    m_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));

    m_iface->create_group("CalculiX-FEA");

    is_initialized = true;  
    return true;
  }
}

bool CoreMaterials::update()
{ 
    return true;
}

bool CoreMaterials::reset()
{
  //init();
  return true;
}

bool CoreMaterials::check_initialized()
{
  return is_initialized;
}