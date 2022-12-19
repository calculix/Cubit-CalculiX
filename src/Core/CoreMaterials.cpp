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
    std::vector<std::string> group_properties_description;
    // group_properties[0][0] property name
    // group_properties[0][1] property name type  1=scalar, 2=vector, 3 = matrix, 4 = tabular
    // group_properties[0][2] type column size for table property

    std::vector<std::string> material_card(2);
    material_card[0]="ELASTIC"; // card name
    material_card[1]="CCX_ELASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ISOTROPIC MATERIAL CARD");
    //group_properties.push_back({material_card[1] + "ISO_MODULUS", "1"});
    //group_properties_description.push_back("ISOTROPIC: Young's Modulus");
    //group_properties.push_back({material_card[1] + "ISO_POISSON", "1"});
    //group_properties_description.push_back("ISOTROPIC: Poisson's ratio");
    group_properties.push_back({material_card[1] + "ISO_MODULUS_VS_POISSON_VS_TEMPERATURE", "4", "3"});
    group_properties_description.push_back("ISOTROPIC:\nYoung's Modulus vs Poisson's ratio vs Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ORTHOTROPIC MATERIAL CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "10"});
    group_properties_description.push_back("ORTHOTROPIC:\nD1111,D1122,D2222,D1133,D2233,D3333,D1212,D1313,\nD2323,TEMPERATURE");
    group_properties.push_back({material_card[1] + "EC_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ENGINEERING CONSTANTS MATERIAL CARD");
    group_properties.push_back({material_card[1] + "EC_CONSTANTS_VS_TEMPERATURE", "4", "10"});
    group_properties_description.push_back("ENGINEERING CONSTANTS:\nE1,E2,E3,v12,v13,v23,G12,G13,\nG23,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ANISO MATERIAL CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "22"});
    group_properties_description.push_back("ANISOTROPIC:\nD1111,D1122,D2222,D1133,D2233,D3333,D1112,D2212,\nD3312,D1212,D1113,D2213,D3313,D1213,D1313,D1123,\nD2223,D3323,D1223,D1323,D2323,TEMPERATURE");
    
    material_card[0]="PLASTIC"; // card name
    material_card[1]="CCX_PLASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC HARDENING=ISOTROPIC MATERIAL CARD");
    group_properties.push_back({material_card[1] + "ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "4","3"});
    group_properties_description.push_back("ISOTROPIC:\nvon Mises Yield Stress, equivalent plastic-strain, Temperature");
    group_properties.push_back({material_card[1] + "KIN_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC HARDENING=KINEMATIC MATERIAL CARD");
    group_properties.push_back({material_card[1] + "KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "4","3"});
    group_properties_description.push_back("KINEMATIC:\nvon Mises Yield Stress, equivalent plastic-strain, Temperature");

    /*
    material_card[0]="DENSITY"; // card name
    material_card[1]="CCX_DENSITY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE", "1"});
    group_properties.push_back({material_card[1] + "DENSITY", "1"});
    */

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
      prop = m_iface->get_property(group_properties[i][0]);
      m_iface->set_property_description(prop,group_properties_description[i]);
      if (group_properties[i][1]=="4"){
        m_iface->set_property_size(prop,std::stoi(group_properties[i][2]));
      }

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

std::string CoreMaterials::get_material_export() // get a list of the CalculiX Material exports
{
  std::vector<std::string> materials_export_list;
  materials_export_list.push_back("*MATERIAL");
  return "*MATERIALS";
}