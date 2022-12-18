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

    std::string group_name = "CalculiX-FEA"; // don't forget to add new groups into grouplist function
    std::vector<std::vector<std::string>> group_properties; 
    // group_properties[0][0] property name
    // group_properties[0][1] property name type  1=scalar, 2=vector, 3 = matrix, 4 = tabular

    std::vector<std::string> material_card(2);
    material_card[0]="ELASTIC"; // card name
    material_card[1]="CCX_ELASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE", "1"});
    group_properties.push_back({material_card[1] + "MODULUS", "1"});
    group_properties.push_back({material_card[1] + "POISSON", "1"});
    
    material_card[0]="PLASTIC"; // card name
    material_card[1]="CCX_PLASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE", "1"});
    group_properties.push_back({material_card[1] + "YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "3"});
    
    material_card[0]="DENSITY"; // card name
    material_card[1]="CCX_DENSITY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE", "1"});
    group_properties.push_back({material_card[1] + "DENSITY", "1"});

    m_iface->create_group(group_name);
    grp = m_iface->get_group(group_name);
    for (size_t i = 0; i < group_properties.size(); i++)
    {
      MaterialInterface::PropertyType prop_type;
       	
      if (group_properties[i][1]=="1")
      {
        prop_type = MaterialInterface::PropertyType::PROP_TYPE_SCALAR;
      }else if (group_properties[i][1]=="2")
      {
        prop_type = MaterialInterface::PropertyType::PROP_TYPE_VECTOR;
      }else if (group_properties[i][1]=="3")
      {
        prop_type = MaterialInterface::PropertyType::PROP_TYPE_MATRIX;
      }else if (group_properties[i][1]=="4")
      {
        prop_type = MaterialInterface::PropertyType::PROP_TYPE_TABULAR;
      }

      prop = m_iface->create_property(group_properties[i][0],prop_type);

      prop = m_iface->get_property(group_properties[i][0]);;
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

std::vector<std::string> CoreMaterials::get_group_list() // get a list of the CalculiX Material Groups
{
  std::vector<std::string> groups;
  groups.push_back("CalculiX-FEA");
  return groups;
}