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
    MaterialInterface::PropertyGroup grp;
    MaterialInterface::Property prop;

    std::string group_name = "CalculiX-FEA";
    std::vector<std::string> group_properties;
    group_properties.push_back("MODULUS");
    group_properties.push_back("POISSON");
    group_properties.push_back("YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE");
    group_properties.push_back("DENSITY");

    m_iface->create_group(group_name);
    grp = m_iface->get_group(group_name);
    for (size_t i = 0; i < group_properties.size(); i++)
    {
      prop = m_iface->get_property(group_properties[i]);;
      m_iface->add_group_property(grp,prop);
    }
    
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

std::vector<std::string> get_group_list() // get a list of the CalculiX Material Groups
{
  std::vector<std::string> groups;
  groups.push_back("CalculiX-FEA");
  return groups;
}