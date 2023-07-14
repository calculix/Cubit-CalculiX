#include "CoreMaterials.hpp"
#include "CubitInterface.hpp"
#include "MaterialInterface.hpp"
#include "CalculiXCoreInterface.hpp"


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
    mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

    MaterialInterface::PropertyGroup grp;
    MaterialInterface::Property prop;

    std::string group_name = "CalculiX-FEA"; // don't forget to add new groups into grouplist function
    group_properties.clear();
    group_properties_description.clear();

    std::vector<std::string> material_card(2);
    material_card[0]="ELASTIC"; // card name
    material_card[1]="CCX_ELASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ISOTROPIC CARD");
    group_properties.push_back({material_card[1] + "ISO_MODULUS_VS_POISSON_VS_TEMPERATURE", "4", "3"});
    group_properties_description.push_back("ISOTROPIC:\nYoung's Modulus vs Poisson's ratio vs Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ORTHOTROPIC CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "10"});
    group_properties_description.push_back("ORTHOTROPIC:\nD1111,D1122,D2222,D1133,D2233,D3333,D1212,D1313,\nD2323,TEMPERATURE");
    group_properties.push_back({material_card[1] + "EC_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ENGINEERING CONSTANTS CARD");
    group_properties.push_back({material_card[1] + "EC_CONSTANTS_VS_TEMPERATURE", "4", "10"});
    group_properties_description.push_back("ENGINEERING CONSTANTS:\nE1,E2,E3,v12,v13,v23,G12,G13,\nG23,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ANISO CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "22"});
    group_properties_description.push_back("ANISOTROPIC:\nD1111,D1122,D2222,D1133,D2233,D3333,D1112,D2212,\nD3312,D1212,D1113,D2213,D3313,D1213,D1313,D1123,\nD2223,D3323,D1223,D1323,D2323,TEMPERATURE");
    
    material_card[0]="PLASTIC"; // card name
    material_card[1]="CCX_PLASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC HARDENING=ISOTROPIC CARD");
    group_properties.push_back({material_card[1] + "ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "4","3"});
    group_properties_description.push_back("ISOTROPIC:\nvon Mises Yield Stress, equivalent plastic-strain, Temperature");
    group_properties.push_back({material_card[1] + "KIN_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC HARDENING=KINEMATIC CARD");
    group_properties.push_back({material_card[1] + "KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "4","3"});
    group_properties_description.push_back("KINEMATIC:\nvon Mises Yield Stress, equivalent plastic-strain, Temperature");

    material_card[0]="DENSITY"; // card name
    material_card[1]="CCX_DENSITY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE DENSITY CARD");
    group_properties.push_back({material_card[1] + "DENSITY", "4","2"});
    group_properties_description.push_back("Density, Temperature");

    material_card[0]="SPECIFIC_HEAT"; // card name
    material_card[1]="CCX_SPECIFIC_HEAT_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE SPECIFIC HEAT CARD");
    group_properties.push_back({material_card[1] + "SPECIFIC_HEAT", "4","2"});
    group_properties_description.push_back("Specific heat, Temperature");

    material_card[0]="EXPANSION"; // card name
    material_card[1]="CCX_EXPANSION_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION TYPE=ISO CARD");
    group_properties.push_back({material_card[1] + "ISO_A_TEMPERATURE", "4", "2"});
    group_properties_description.push_back("ISOTROPIC:\n \u03B1,Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION TYPE=ORTHO CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "4"});
    group_properties_description.push_back("ORTHOTROPIC:\n\u03B111,\u03B122,\u03B133,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION TYPE=ANISO CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "7"});
    group_properties_description.push_back("ANISOTROPIC:\n\u03B111,\u03B122,\u03B133,\u03B112,\u03B113,\u03B123,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ZERO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION ZERO=TEMP CARD");
    group_properties.push_back({material_card[1] + "ZERO", "1"});
    group_properties_description.push_back("ZERO Temperature");

    material_card[0]="CONDUCTIVITY"; // card name
    material_card[1]="CCX_CONDUCTIVITY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CONDUCTIVITY CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CONDUCTIVITY TYPE=ISO CARD");
    group_properties.push_back({material_card[1] + "ISO_K_TEMPERATURE", "4", "2"});
    group_properties_description.push_back("ISOTROPIC:\n \u03BA,Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CONDUCTIVITY TYPE=ORTHO CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "4"});
    group_properties_description.push_back("ORTHOTROPIC:\n\u03BA11,\u03BA22,\u03BA33,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CONDUCTIVITY TYPE=ANISO CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "7"});
    group_properties_description.push_back("ANISOTROPIC:\n\u03BA11,\u03BA22,\u03BA33,\u03BA12,\u03BA13,\u03BA23,TEMPERATURE");

    mat_iface->create_group(group_name);
    grp = mat_iface->get_group(group_name);
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

      prop = mat_iface->create_property(group_properties[i][0],prop_type);
      prop = mat_iface->get_property(group_properties[i][0]);
      mat_iface->set_property_description(prop,group_properties_description[i]);
      if (group_properties[i][1]=="4"){
        mat_iface->set_property_size(prop,std::stoi(group_properties[i][2]));
      }

      mat_iface->add_group_property(grp,prop);
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
  materials_export_list.push_back("********************************** M A T E R I A L S ****************************");

  std::vector<std::string> material_name_list;
  material_name_list = CubitInterface::get_material_name_list();
  //loop over all materials
  for (size_t i = 0; i < material_name_list.size(); i++)
  { 
    MaterialInterface::Material material;
    MaterialInterface::PropertyGroup group;
    std::string group_name;
    std::vector<std::string> group_list;
    material = mat_iface->get_material(material_name_list[i]);
    group = mat_iface->get_material_property_group(material);
    group_name = mat_iface->get_group_name(group);
    group_list = this->get_group_list();

    if (std::find(group_list.begin(), group_list.end(), group_name) != group_list.end())
    {
      // CUSTOMLINE START
      std::vector<std::string> customline = ccx_iface->get_customline_data("BEFORE","MATERIAL",mat_iface->get_material_id(material));
      for (size_t icl = 0; icl < customline.size(); icl++)
      {
        materials_export_list.push_back(customline[icl]);
      }
      // CUSTOMLINE END

      materials_export_list.push_back("*MATERIAL, NAME=" + material_name_list[i]); //material_name
      materials_export_list.push_back(this->get_material_cards_export(material_name_list[i],group_name));

      // CUSTOMLINE START
      customline = ccx_iface->get_customline_data("AFTER","MATERIAL",mat_iface->get_material_id(material));
      for (size_t icl = 0; icl < customline.size(); icl++)
      {
        materials_export_list.push_back(customline[icl]);
      }
      // CUSTOMLINE END
    }
  }

  std::string material_export;

  for (size_t i = 0; i < materials_export_list.size(); i++)
  {
    material_export.append(materials_export_list[i] + "\n");
  }
  
  return material_export;
}

std::string CoreMaterials::get_material_cards_export(std::string material_name, std::string group_name)
{ 
  MaterialInterface::Material material;
  MaterialInterface::Property prop;
  material = mat_iface->get_material(material_name);
  double prop_scalar;
  MaterialVector prop_vector;
  MaterialMatrix prop_matrix;
  std::vector<std::string> material_card(2);
  std::string material_cards_export;

  if (group_name=="CalculiX-FEA")
  {
    // check if card is needed and then process the properties
    
    material_card[0]="ELASTIC"; // card name
    material_card[1]="CCX_ELASTIC_"; // property prefix

    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    { 
      if (prop_scalar==1)
      { 
        prop = mat_iface->get_property(material_card[1] + "ISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*ELASTIC,TYPE=ISO\n");

            prop = mat_iface->get_property(material_card[1] + "ISO_MODULUS_VS_POISSON_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
            }
          }
        }
        prop = mat_iface->get_property(material_card[1] + "ORTHO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*ELASTIC,TYPE=ORTHO\n");

            prop = mat_iface->get_property(material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][6]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][7]) + ",\n");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][8]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][9]) + "\n");
            }
          }
        }
        prop = mat_iface->get_property(material_card[1] + "EC_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*ELASTIC,TYPE=ENGINEERING CONSTANTS\n");

            prop = mat_iface->get_property(material_card[1] + "EC_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][6]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][7]) + ",\n");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][8]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][9]) + "\n");
            }
          }
        }
        prop = mat_iface->get_property(material_card[1] + "ANISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*ELASTIC,TYPE=ANISO\n");

            prop = mat_iface->get_property(material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][6]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][7]) + ",\n");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][8]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][9]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][10]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][11]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][12]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][13]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][14]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][15]) + ",\n");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][16]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][17]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][18]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][19]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][20]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][21]) + "\n");
            }
          }
        }
      }
    }
    material_card[0]="PLASTIC"; // card name
    material_card[1]="CCX_PLASTIC_"; // property prefix
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    { 
      if (prop_scalar==1)
      {       
        prop = mat_iface->get_property(material_card[1] + "ISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*PLASTIC,HARDENING=ISOTROPIC\n");

            prop = mat_iface->get_property(material_card[1] + "ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
            }
          }
        }
        prop = mat_iface->get_property(material_card[1] + "KIN_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*PLASTIC,HARDENING=KINEMATIC\n");

            prop = mat_iface->get_property(material_card[1] + "KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
            }
          }
        }
      }
    }

    material_card[0]="DENSITY"; // card name
    material_card[1]="CCX_DENSITY_"; // property prefix

    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*DENSITY\n");

        prop = mat_iface->get_property(material_card[1] + "DENSITY");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + "\n");
        }
      }
    }

    material_card[0]="SPECIFIC_HEAT"; // card name
    material_card[1]="CCX_SPECIFIC_HEAT_"; // property prefix

    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*SPECIFIC HEAT\n");

        prop = mat_iface->get_property(material_card[1] + "SPECIFIC_HEAT");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + "\n");
        }
      }
    }

    material_card[0]="EXPANSION"; // card name
    material_card[1]="CCX_EXPANSION_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    { 
      if (prop_scalar==1)
      { 
        prop = mat_iface->get_property(material_card[1] + "ISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            MaterialInterface::Property prop_zero;
            prop_zero = mat_iface->get_property(material_card[1] + "ZERO_USE_CARD");
            if (mat_iface->get_material_property_value(material, prop_zero, prop_scalar))
            {
              material_cards_export.append("*EXPANSION,TYPE=ISO,ZERO=" + ccx_iface->to_string_scientific(prop_scalar) + "\n");
            }else
            {
              material_cards_export.append("*EXPANSION,TYPE=ISO\n");
            }

            prop = mat_iface->get_property(material_card[1] + "ISO_A_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "ORTHO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            MaterialInterface::Property prop_zero;
            prop_zero = mat_iface->get_property(material_card[1] + "ZERO_USE_CARD");
            if (mat_iface->get_material_property_value(material, prop_zero, prop_scalar))
            {
              material_cards_export.append("*EXPANSION,TYPE=ORTHO,ZERO=" + ccx_iface->to_string_scientific(prop_scalar) + "\n");
            }else
            {
              material_cards_export.append("*EXPANSION,TYPE=ORTHO\n");
            }    

            prop = mat_iface->get_property(material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + "\n");
            }
          }
        }
        
        prop = mat_iface->get_property(material_card[1] + "ANISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          { 
            MaterialInterface::Property prop_zero;
            prop_zero = mat_iface->get_property(material_card[1] + "ZERO_USE_CARD");
            if (mat_iface->get_material_property_value(material, prop_zero, prop_scalar))
            {
              material_cards_export.append("*EXPANSION,TYPE=ANISO,ZERO=" + ccx_iface->to_string_scientific(prop_scalar) + "\n");
            }else
            {
              material_cards_export.append("*EXPANSION,TYPE=ANISO\n");
            }  

            prop = mat_iface->get_property(material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][6]) + "\n");
            }
          }
        }
      }
    }

    material_card[0]="CONDUCTIVITY"; // card name
    material_card[1]="CCX_CONDUCTIVITY_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    { 
      if (prop_scalar==1)
      { 
        prop = mat_iface->get_property(material_card[1] + "ISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            
            material_cards_export.append("*CONDUCTIVITY,TYPE=ISO\n");
            
            prop = mat_iface->get_property(material_card[1] + "ISO_A_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "ORTHO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*CONDUCTIVITY,TYPE=ORTHO\n");    

            prop = mat_iface->get_property(material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + "\n");
            }
          }
        }
        
        prop = mat_iface->get_property(material_card[1] + "ANISO_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          { 
            material_cards_export.append("*CONDUCTIVITY,TYPE=ANISO\n");

            prop = mat_iface->get_property(material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][6]) + "\n");
            }
          }
        }
      }
    }
  } 

  return material_cards_export;
}

bool CoreMaterials::check_material_exists(int material_id)
{
  MaterialInterface::Material material;
  material = mat_iface->get_material(material_id);
  if (material != NULL)
  {
    return true;
  }
  return false;
}