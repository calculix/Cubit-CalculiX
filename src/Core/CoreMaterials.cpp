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
    group_properties.push_back({material_card[1] + "COMBINED_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC HARDENING=COMBINED CARD");
    group_properties.push_back({material_card[1] + "COMBINED_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "4","3"});
    group_properties_description.push_back("COMBINED:\nvon Mises Yield Stress, equivalent plastic-strain, Temperature");
    group_properties.push_back({material_card[1] + "JOHNSON_COOK_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE PLASTIC HARDENING=JOHNSON COOK CARD");
    group_properties.push_back({material_card[1] + "JOHNSON_COOK", "4","6"});
    group_properties_description.push_back("JOHNSON_COOK:\nA,B,n,m,Tm,T0");

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
    group_properties_description.push_back(u8"ISOTROPIC:\n \u03B1,Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION TYPE=ORTHO CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "4"});
    group_properties_description.push_back(u8"ORTHOTROPIC:\n\u03B111,\u03B122,\u03B133,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE EXPANSION TYPE=ANISO CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "7"});
    group_properties_description.push_back(u8"ANISOTROPIC:\n\u03B111,\u03B122,\u03B133,\u03B112,\u03B113,\u03B123,TEMPERATURE");
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
    group_properties_description.push_back(u8"ISOTROPIC:\n \u03BA,Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CONDUCTIVITY TYPE=ORTHO CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "4"});
    group_properties_description.push_back(u8"ORTHOTROPIC:\n\u03BA11,\u03BA22,\u03BA33,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CONDUCTIVITY TYPE=ANISO CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "7"});
    group_properties_description.push_back(u8"ANISOTROPIC:\n\u03BA11,\u03BA22,\u03BA33,\u03BA12,\u03BA13,\u03BA23,TEMPERATURE");

    material_card[0]="DEFORMATION_PLASTICITY"; // card name
    material_card[1]="CCX_DEFORMATION_PLASTICITY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE DEFORMATION PLASTICITY CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "RAMBERG_OSGOOD", "4", "6"});
    group_properties_description.push_back(u8"RAMBERG OSGOOD:\nYoung's Modulus, Poisson's ratio, Yield Stress, Exponent, Yield Offset, Temperature");

    material_card[0]="MOHR_COULOMB"; // card name
    material_card[1]="CCX_MOHR_COULOMB_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE MOHR COULOMB CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "MOHR_COULOMB", "4", "3"});
    group_properties_description.push_back(u8"MOHR COULOMB:\nFriction angle φ in degrees, Dilation angle ψ in degrees, Temperature");

    material_card[0]="MOHR_COULOMB_HARDENING"; // card name
    material_card[1]="CCX_MOHR_COULOMB_HARDENING_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE MOHR COULOMB HARDENING CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "MOHR_COULOMB_HARDENING", "4", "3"});
    group_properties_description.push_back(u8"MOHR COULOMB:\nCohesion yield stress c, Equivalent plastic strain, Temperature");

    material_card[0]="CYCLIC_HARDENING"; // card name
    material_card[1]="CCX_CYCLIC_HARDENING_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CYCLIC HARDENING CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "CYCLIC_HARDENING", "4", "3"});
    group_properties_description.push_back(u8"CYCLIC HARDENING:\nVon Mises stress, Equivalent plastic strain, Temperature");

    material_card[0]="RATE_DEPENDENT"; // card name
    material_card[1]="CCX_RATE_DEPENDENT_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE RATE DEPENDENT CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "RATE_DEPENDENT", "4", "2"});
    group_properties_description.push_back(u8"RATE DEPENDENT:\nC, \u03B5_dot_0");

    material_card[0]="CREEP"; // card name
    material_card[1]="CCX_CREEP_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE CREEP CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "CREEP", "4", "4"});
    group_properties_description.push_back(u8"CREEP:\nA,n,m,Temperature");

    material_card[0]="HYPERELASTIC"; // card name
    material_card[1]="CCX_HYPERELASTIC_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "ARRUDA_BOYCE_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC ARRUDA BOYCE CARD");
    group_properties.push_back({material_card[1] + "ARRUDA_BOYCE", "4", "4"});
    group_properties_description.push_back("ARRUDA BOYCE:\n\u03BC,\u03BBm,D, Temperature");
    group_properties.push_back({material_card[1] + "MOONEY_RIVLIN_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC MOONEY RIVLIN CARD");
    group_properties.push_back({material_card[1] + "MOONEY_RIVLIN", "4", "4"});
    group_properties_description.push_back("MOONEY RIVLIN:\nC10,C01,D1, Temperature");
    group_properties.push_back({material_card[1] + "NEO_HOOKE_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC NEO HOOKE CARD");
    group_properties.push_back({material_card[1] + "NEO_HOOKE", "4", "3"});
    group_properties_description.push_back("NEO HOOKE:\nC10,D1, Temperature");
    group_properties.push_back({material_card[1] + "OGDEN_N1_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC OGDEN N1 CARD");
    group_properties.push_back({material_card[1] + "OGDEN_N1", "4", "4"});
    group_properties_description.push_back("OGDEN N1:\n\u03BC1,\u03B11,D1, Temperature");
    group_properties.push_back({material_card[1] + "OGDEN_N2_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC OGDEN N2 CARD");
    group_properties.push_back({material_card[1] + "OGDEN_N2", "4", "7"});
    group_properties_description.push_back("OGDEN N2:\n\u03BC1,\u03B11,D1,\u03BC2,\u03B12,D2, Temperature");
    group_properties.push_back({material_card[1] + "OGDEN_N3_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC OGDEN N3 CARD");
    group_properties.push_back({material_card[1] + "OGDEN_N3", "4", "10"});
    group_properties_description.push_back("OGDEN N3:\n\u03BC1,\u03B11,D1,\u03BC2,\u03B12,D2,\u03BC3,\u03B13,D3, Temperature");
    group_properties.push_back({material_card[1] + "POLYNOMIAL_N1_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC POLYNOMIAL N1 CARD");
    group_properties.push_back({material_card[1] + "POLYNOMIAL_N1", "4", "4"});
    group_properties_description.push_back("POLYNOMIAL N1:\nC10,C01,D1, Temperature");
    group_properties.push_back({material_card[1] + "POLYNOMIAL_N2_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC POLYNOMIAL N2 CARD");
    group_properties.push_back({material_card[1] + "POLYNOMIAL_N2", "4", "8"});
    group_properties_description.push_back("POLYNOMIAL N2:\nC10,C01,C20,C11,C02,D1,D2, Temperature");
    group_properties.push_back({material_card[1] + "POLYNOMIAL_N3_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC POLYNOMIAL N3 CARD");
    group_properties.push_back({material_card[1] + "POLYNOMIAL_N3", "4", "13"});
    group_properties_description.push_back("POLYNOMIAL N3:\nC10,C01,C20,C11,C02,C30,C21,C12,C03,D1,D2,D3, Temperature");
    group_properties.push_back({material_card[1] + "REDUCED_POLYNOMIAL_N1_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC REDUCED POLYNOMIAL N1 CARD");
    group_properties.push_back({material_card[1] + "REDUCED_POLYNOMIAL_N1", "4", "3"});
    group_properties_description.push_back("REDUCED POLYNOMIAL N1:\nC10,D1, Temperature");
    group_properties.push_back({material_card[1] + "REDUCED_POLYNOMIAL_N2_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC REDUCED POLYNOMIAL N2 CARD");
    group_properties.push_back({material_card[1] + "REDUCED_POLYNOMIAL_N2", "4", "5"});
    group_properties_description.push_back("REDUCED POLYNOMIAL N2:\nC10,C20,D1,D2, Temperature");
    group_properties.push_back({material_card[1] + "REDUCED_POLYNOMIAL_N3_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC REDUCED POLYNOMIAL N3 CARD");
    group_properties.push_back({material_card[1] + "REDUCED_POLYNOMIAL_N3", "4", "7"});
    group_properties_description.push_back("REDUCED POLYNOMIAL N3:\nC10,C20,C30,D1,D2,D3, Temperature");
    group_properties.push_back({material_card[1] + "YEOH_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERELASTIC YEOH CARD");
    group_properties.push_back({material_card[1] + "YEOH", "4", "7"});
    group_properties_description.push_back("YEOH:\nC10,C20,C30,D1,D2,D3, Temperature");
    
    material_card[0]="HYPERFOAM"; // card name
    material_card[1]="CCX_HYPERFOAM_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERFOAM CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "HYPERFOAM_N1_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERFOAM N1 CARD");
    group_properties.push_back({material_card[1] + "HYPERFOAM_N1", "4", "4"});
    group_properties_description.push_back("HYPERFOAM N1:\n\u03BC1,\u03B11,\u03BD1, Temperature");
    group_properties.push_back({material_card[1] + "HYPERFOAM_N2_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERFOAM N2 CARD");
    group_properties.push_back({material_card[1] + "HYPERFOAM_N2", "4", "7"});
    group_properties_description.push_back("HYPERFOAM N2:\n\u03BC1,\u03B11,\u03BC2,\u03B12,\u03BD1,\u03BD2, Temperature");
    group_properties.push_back({material_card[1] + "HYPERFOAM_N3_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE HYPERFOAM N3 CARD");
    group_properties.push_back({material_card[1] + "HYPERFOAM_N3", "4", "10"});
    group_properties_description.push_back("HYPERFOAM N3:\n\u03BC1,\u03B11,\u03BC2,\u03B12,\u03BC3,\u03B13,\u03BD1,\u03BD2,\u03BD3, Temperature");
    
    material_card[0]="COMPRESSION_ONLY"; // card name
    material_card[1]="CCX_COMPRESSION_ONLY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE COMPRESSION ONLY CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "COMPRESSION_ONLY", "4", "3"});
    group_properties_description.push_back(u8"COMPRESSION ONLY:\nE,maximum allowed value,Temperature");

    material_card[0]="TENSION_ONLY"; // card name
    material_card[1]="CCX_TENSION_ONLY_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE TENSION ONLY CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "TENSION_ONLY", "4", "3"});
    group_properties_description.push_back(u8"TENSION ONLY:\nE,maximum allowed value,Temperature");

    material_card[0]="DAMPING"; // card name
    material_card[1]="CCX_DAMPING_"; // property prefix
    material_cards.push_back(material_card);
    group_properties.push_back({material_card[1] + "USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE DAMPING CARD"); // needed for Material Management GUI
    group_properties.push_back({material_card[1] + "DAMPING", "1"});
    group_properties_description.push_back(u8"DAMPING:\nStructural");

    //*ELECTRICAL CONDUCTIVITY
    //*MAGNETIC PERMEABILITY

    //*FLUID CONSTANTS
    //*SPECIFIC GAS CONSTANT

    //*USER MATERIAL
    //*DEPVAR

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

int CoreMaterials::get_group_property_size(std::string group_property)
{
  for (size_t i = 0; i < group_properties.size(); i++)
  { 
    if (group_properties[i][0]==group_property)
    { 
      if (group_properties[i][1]=="1")
      {
        return 1;
      }else if (group_properties[i][1]=="2")
      {
      }else if (group_properties[i][1]=="3")
      {
      }else if (group_properties[i][1]=="4")
      {
        return std::stoi(group_properties[i][2]);
      }
    }
  }

  return -1;
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

      //check if name needs to be changed, like for compression only
      std::string prefix = this->get_material_prefix(material_name_list[i]);

      materials_export_list.push_back("*MATERIAL, NAME=" + prefix + material_name_list[i]); //material_name
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
        prop = mat_iface->get_property(material_card[1] + "COMBINED_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*PLASTIC,HARDENING=COMBINED\n");

            prop = mat_iface->get_property(material_card[1] + "COMBINED_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
            }
          }
        }
        prop = mat_iface->get_property(material_card[1] + "JOHNSON_COOK_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*PLASTIC,HARDENING=JOHNSON COOK\n");

            prop = mat_iface->get_property(material_card[1] + "JOHNSON_COOK");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + "\n");
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
            
            prop = mat_iface->get_property(material_card[1] + "ISO_K_TEMPERATURE");
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

    material_card[0]="DEFORMATION_PLASTICITY"; // card name
    material_card[1]="CCX_DEFORMATION_PLASTICITY_"; // property prefix

    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*DEFORMATION PLASTICITY\n");

        prop = mat_iface->get_property(material_card[1] + "RAMBERG_OSGOOD");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][5]) + "\n");
        }
      }
    }

    material_card[0]="MOHR_COULOMB"; // card name
    material_card[1]="CCX_MOHR_COULOMB_"; // property prefix

    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*MOHR COULOMB\n");

        prop = mat_iface->get_property(material_card[1] + "MOHR_COULOMB");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
        }
      }
    }

    material_card[0]="MOHR_COULOMB_HARDENING"; // card name
    material_card[1]="CCX_MOHR_COULOMB_HARDENING_"; // property prefix
   
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*MOHR COULOMB HARDENING\n");

        prop = mat_iface->get_property(material_card[1] + "MOHR_COULOMB_HARDENING");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
        }
      }
    }

    material_card[0]="CYCLIC_HARDENING"; // card name
    material_card[1]="CCX_CYCLIC_HARDENING_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*CYCLIC HARDENING\n");

        prop = mat_iface->get_property(material_card[1] + "CYCLIC_HARDENING");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
        }
      }
    }

    material_card[0]="RATE_DEPENDENT"; // card name
    material_card[1]="CCX_RATE_DEPENDENT_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*RATE DEPENDENT,TYPE=JOHNSON COOK\n");

        prop = mat_iface->get_property(material_card[1] + "RATE_DEPENDENT");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + "\n");
        }
      }
    }

    material_card[0]="CREEP"; // card name
    material_card[1]="CCX_CREEP_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*CREEP,LAW=NORTON\n");

        prop = mat_iface->get_property(material_card[1] + "CREEP");
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
    
    material_card[0]="HYPERELASTIC"; // card name
    material_card[1]="CCX_HYPERELASTIC_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    { 
      if (prop_scalar==1)
      { 
        prop = mat_iface->get_property(material_card[1] + "ARRUDA_BOYCE_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,ARRUDA-BOYCE\n");

            prop = mat_iface->get_property(material_card[1] + "ARRUDA_BOYCE");
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

        prop = mat_iface->get_property(material_card[1] + "MOONEY_RIVLIN_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,MOONEY-RIVLIN\n");

            prop = mat_iface->get_property(material_card[1] + "MOONEY_RIVLIN");
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

        prop = mat_iface->get_property(material_card[1] + "NEO_HOOKE_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,NEO HOOKE\n");

            prop = mat_iface->get_property(material_card[1] + "NEO_HOOKE");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "OGDEN_N1_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,OGDEN,N=1\n");

            prop = mat_iface->get_property(material_card[1] + "OGDEN_N1");
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

        prop = mat_iface->get_property(material_card[1] + "OGDEN_N2_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,OGDEN,N=2\n");

            prop = mat_iface->get_property(material_card[1] + "OGDEN_N2");
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

        prop = mat_iface->get_property(material_card[1] + "OGDEN_N3_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,OGDEN,N=3\n");

            prop = mat_iface->get_property(material_card[1] + "OGDEN_N3");
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

        prop = mat_iface->get_property(material_card[1] + "POLYNOMIAL_N1_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,POLYNOMIAL,N=1\n");

            prop = mat_iface->get_property(material_card[1] + "POLYNOMIAL_N1");
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

        prop = mat_iface->get_property(material_card[1] + "POLYNOMIAL_N2_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,POLYNOMIAL,N=2\n");

            prop = mat_iface->get_property(material_card[1] + "POLYNOMIAL_N2");
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
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][7]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "POLYNOMIAL_N3_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,POLYNOMIAL,N=3\n");

            prop = mat_iface->get_property(material_card[1] + "POLYNOMIAL_N3");
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
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][12]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "REDUCED_POLYNOMIAL_N1_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,REDUCED_POLYNOMIAL,N=1\n");

            prop = mat_iface->get_property(material_card[1] + "REDUCED_POLYNOMIAL_N1");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "REDUCED_POLYNOMIAL_N2_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,REDUCED_POLYNOMIAL,N=2\n");

            prop = mat_iface->get_property(material_card[1] + "REDUCED_POLYNOMIAL_N2");
            mat_iface->get_material_property_value(material, prop, prop_matrix);

            for (size_t i = 0; i < prop_matrix.size(); i++)
            {
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][3]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][4]) + "\n");
            }
          }
        }

        prop = mat_iface->get_property(material_card[1] + "REDUCED_POLYNOMIAL_N3_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,REDUCED_POLYNOMIAL,N=3\n");

            prop = mat_iface->get_property(material_card[1] + "REDUCED_POLYNOMIAL_N3");
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

        prop = mat_iface->get_property(material_card[1] + "YEOH_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERELASTIC,YEOH\n");

            prop = mat_iface->get_property(material_card[1] + "YEOH");
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
    
    
    material_card[0]="HYPERFOAM"; // card name
    material_card[1]="CCX_HYPERFOAM_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        prop = mat_iface->get_property(material_card[1] + "HYPERFOAM_N1_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERFOAM,N=1\n");

            prop = mat_iface->get_property(material_card[1] + "HYPERFOAM_N1");
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

        prop = mat_iface->get_property(material_card[1] + "HYPERFOAM_N2_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERFOAM,N=2\n");

            prop = mat_iface->get_property(material_card[1] + "HYPERFOAM_N2");
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

        prop = mat_iface->get_property(material_card[1] + "HYPERFOAM_N3_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            material_cards_export.append("*HYPERFOAM,N=3\n");

            prop = mat_iface->get_property(material_card[1] + "HYPERFOAM_N3");
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
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][7]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][8]) + ",");
              material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][9]) + "\n");
            }
          }
        }
      }
    }

    material_card[0]="COMPRESSION_ONLY"; // card name
    material_card[1]="CCX_COMPRESSION_ONLY_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*USER MATERIAL,CONSTANTS=2\n");

        prop = mat_iface->get_property(material_card[1] + "COMPRESSION_ONLY");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
        }
      }
    }
 
    material_card[0]="TENSION_ONLY"; // card name
    material_card[1]="CCX_TENSION_ONLY_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        material_cards_export.append("*USER MATERIAL,CONSTANTS=2\n");

        prop = mat_iface->get_property(material_card[1] + "TENSION_ONLY");
        mat_iface->get_material_property_value(material, prop, prop_matrix);

        for (size_t i = 0; i < prop_matrix.size(); i++)
        {
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][0]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][1]) + ",");
          material_cards_export.append(ccx_iface->to_string_scientific(prop_matrix[i][2]) + "\n");
        }
      }
    }

    material_card[0]="DAMPING"; // card name
    material_card[1]="CCX_DAMPING_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        prop = mat_iface->get_property(material_card[1] + "DAMPING");
        mat_iface->get_material_property_value(material, prop, prop_scalar);
        material_cards_export.append("*DAMPING,STRUCTURAL=" + std::to_string(prop_scalar) + "\n");
      }
    }

    //*ELECTRICAL CONDUCTIVITY
    //*MAGNETIC PERMEABILITY


    //*FLUID CONSTANTS
    //*SPECIFIC GAS CONSTANT

    //*USER MATERIAL
    //*DEPVAR

  } 
  return material_cards_export;
}

std::string CoreMaterials::get_material_prefix(std::string material_name)
{ 
  MaterialInterface::Material material;
  MaterialInterface::PropertyGroup group;
  std::string group_name;
  MaterialInterface::Property prop;
  material = mat_iface->get_material(material_name);
  group = mat_iface->get_material_property_group(material);
  group_name = mat_iface->get_group_name(group);
  double prop_scalar;
  MaterialVector prop_vector;
  MaterialMatrix prop_matrix;
  std::vector<std::string> material_card(2);
  std::string prefix="";  

  if (group_name=="CalculiX-FEA")
  {
    // check if card is needed and then process the properties
    material_card[0]="COMPRESSION_ONLY"; // card name
    material_card[1]="CCX_COMPRESSION_ONLY_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        prefix="COMPRESSION_ONLY_";
        return prefix;
      }
    }
    material_card[0]="TENSION_ONLY"; // card name
    material_card[1]="CCX_TENSION_ONLY_"; // property prefix
    
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    {
      if (prop_scalar==1)
      {
        prefix="TENSION_ONLY_";
        return prefix;
      }
    }

    material_card[0]="PLASTIC"; // card name
    material_card[1]="CCX_PLASTIC_"; // property prefix
    prop = mat_iface->get_property(material_card[1] + "USE_CARD");
    if (mat_iface->get_material_property_value(material, prop, prop_scalar))
    { 
      if (prop_scalar==1)
      {       
        prop = mat_iface->get_property(material_card[1] + "JOHNSON_COOK_USE_CARD");
        if (mat_iface->get_material_property_value(material, prop, prop_scalar))
        {
          if (prop_scalar==1)
          {
            prefix="JOHNSONCOOK_";
          return prefix;  
          }
        }
      }
    }
 
  } 
  return prefix;
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