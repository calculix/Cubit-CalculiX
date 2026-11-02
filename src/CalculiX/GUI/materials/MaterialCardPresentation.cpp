#include "MaterialCardPresentation.hpp"
#include <algorithm>

namespace
{
const std::vector<MaterialCardDefinition>& definitions()
{
  static const std::vector<MaterialCardDefinition> cards = {
    {"CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE", "Elasticity", "Isotropic", "CCX_ELASTIC_USE_CARD", "CCX_ELASTIC_ISO_USE_CARD", {"Young's Modulus","Poisson's ratio","Temperature"}},
    {"CCX_ELASTIC_ORTHO_CONSTANTS_VS_TEMPERATURE", "Elasticity", "Orthotropic", "CCX_ELASTIC_USE_CARD", "CCX_ELASTIC_ORTHO_USE_CARD", {"D1111","D1122","D2222","D1133","D2233","D3333","D1212","D1313","D2323","TEMPERATURE"}},
    {"CCX_ELASTIC_EC_CONSTANTS_VS_TEMPERATURE", "Elasticity", "Engineering constants", "CCX_ELASTIC_USE_CARD", "CCX_ELASTIC_EC_USE_CARD", {"E1","E2","E3","v12","v13","v23","G12","G13","G23","TEMPERATURE"}},
    {"CCX_ELASTIC_ANISO_CONSTANTS_VS_TEMPERATURE", "Elasticity", "Anisotropic", "CCX_ELASTIC_USE_CARD", "CCX_ELASTIC_ANISO_USE_CARD", {"D1111","D1122","D2222","D1133","D2233","D3333","D1112","D2212","D3312","D1212","D1113","D2213","D3313","D1213","D1313","D1123","D2223","D3323","D1223","D1323","D2323","TEMPERATURE"}},
    {"CCX_PLASTIC_ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "Plasticity", "Isotropic", "CCX_PLASTIC_USE_CARD", "CCX_PLASTIC_ISO_USE_CARD", {"von Mises Stress","Plastic Strain","Temperature"}},
    {"CCX_PLASTIC_KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "Plasticity", "Kinematic", "CCX_PLASTIC_USE_CARD", "CCX_PLASTIC_KIN_USE_CARD", {"von Mises Stress","Plastic Strain","Temperature"}},
    {"CCX_PLASTIC_COMBINED_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "Plasticity", "Combined", "CCX_PLASTIC_USE_CARD", "CCX_PLASTIC_COMBINED_USE_CARD", {"von Mises Stress","Plastic Strain","Temperature"}},
    {"CCX_PLASTIC_JOHNSON_COOK", "Plasticity", "Johnson-Cook", "CCX_PLASTIC_USE_CARD", "CCX_PLASTIC_JOHNSON_COOK_USE_CARD", {"A","B","n","m","Tm","T0"}},
    {"CCX_DENSITY_DENSITY", "Density", "", "CCX_DENSITY_USE_CARD", "", {"Density","Temperature"}},
    {"CCX_SPECIFIC_HEAT_SPECIFIC_HEAT", "Specific heat", "", "CCX_SPECIFIC_HEAT_USE_CARD", "", {"Specific heat","Temperature"}},
    {"CCX_EXPANSION_ISO_A_TEMPERATURE", "Expansion", "Isotropic", "CCX_EXPANSION_USE_CARD", "CCX_EXPANSION_ISO_USE_CARD", {u8"\u03B1","Temperature"}},
    {"CCX_EXPANSION_ORTHO_CONSTANTS_VS_TEMPERATURE", "Expansion", "Orthotropic", "CCX_EXPANSION_USE_CARD", "CCX_EXPANSION_ORTHO_USE_CARD", {u8"\u03B111",u8"\u03B122",u8"\u03B133","Temperature"}},
    {"CCX_EXPANSION_ANISO_CONSTANTS_VS_TEMPERATURE", "Expansion", "Anisotropic", "CCX_EXPANSION_USE_CARD", "CCX_EXPANSION_ANISO_USE_CARD", {u8"\u03B111",u8"\u03B122",u8"\u03B133",u8"\u03B112",u8"\u03B113",u8"\u03B123","Temperature"}},
    {"CCX_CONDUCTIVITY_ISO_K_TEMPERATURE", "Conductivity", "Isotropic", "CCX_CONDUCTIVITY_USE_CARD", "CCX_CONDUCTIVITY_ISO_USE_CARD", {u8"\u03BA","Temperature"}},
    {"CCX_CONDUCTIVITY_ORTHO_CONSTANTS_VS_TEMPERATURE", "Conductivity", "Orthotropic", "CCX_CONDUCTIVITY_USE_CARD", "CCX_CONDUCTIVITY_ORTHO_USE_CARD", {u8"\u03BA11",u8"\u03BA22",u8"\u03BA33","Temperature"}},
    {"CCX_CONDUCTIVITY_ANISO_CONSTANTS_VS_TEMPERATURE", "Conductivity", "Anisotropic", "CCX_CONDUCTIVITY_USE_CARD", "CCX_CONDUCTIVITY_ANISO_USE_CARD", {u8"\u03BA11",u8"\u03BA22",u8"\u03BA33",u8"\u03BA12",u8"\u03BA13",u8"\u03BA23","Temperature"}},
    {"CCX_DEFORMATION_PLASTICITY_RAMBERG_OSGOOD", "Deformation plasticity", "Ramberg-Osgood", "CCX_DEFORMATION_PLASTICITY_USE_CARD", "", {"Young's Modulus", "Poisson's ratio", "Yield Stress", "Exponent", "Yield Offset","Temperature"}},
    {"CCX_MOHR_COULOMB_MOHR_COULOMB", "Mohr-Coulomb", "", "CCX_MOHR_COULOMB_USE_CARD", "", {"Friction angle", "Dilation angle","Temperature"}},
    {"CCX_MOHR_COULOMB_HARDENING_MOHR_COULOMB_HARDENING", "Mohr-Coulomb hardening", "", "CCX_MOHR_COULOMB_HARDENING_USE_CARD", "", {"Cohesion yield stress c", "Equivalent plastic strain","Temperature"}},
    {"CCX_CYCLIC_HARDENING_CYCLIC_HARDENING", "Cyclic hardening", "", "CCX_CYCLIC_HARDENING_USE_CARD", "", {"Von Mises stress", "Equivalent plastic strain","Temperature"}},
    {"CCX_RATE_DEPENDENT_RATE_DEPENDENT", "Rate dependence", "", "CCX_RATE_DEPENDENT_USE_CARD", "", {"C", u8"\u03B5_dot_0"}},
    {"CCX_CREEP_CREEP", "Creep", "", "CCX_CREEP_USE_CARD", "", {"A","n","m","Temperature"}},
    {"CCX_HYPERELASTIC_ARRUDA_BOYCE", "Hyperelasticity", "Arruda Boyce", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", {u8"\u03BC",u8"\u03BBm","D","Temperature"}},
    {"CCX_HYPERELASTIC_MOONEY_RIVLIN", "Hyperelasticity", "Mooney Rivlin", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", {"C10","C01","D1","Temperature"}},
    {"CCX_HYPERELASTIC_NEO_HOOKE", "Hyperelasticity", "Neo Hooke", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", {"C10","D1","Temperature"}},
    {"CCX_HYPERELASTIC_OGDEN_N1", "Hyperelasticity", "Ogden N=1", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", {u8"\u03BC1",u8"\u03B11","D1","Temperature"}},
    {"CCX_HYPERELASTIC_OGDEN_N2", "Hyperelasticity", "Ogden N=2", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", {u8"\u03BC1",u8"\u03B11","D1",u8"\u03BC2",u8"\u03B12","D2","Temperature"}},
    {"CCX_HYPERELASTIC_OGDEN_N3", "Hyperelasticity", "Ogden N=3", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", {u8"\u03BC1",u8"\u03B11","D1",u8"\u03BC2",u8"\u03B12","D2",u8"\u03BC3",u8"\u03B13","D3","Temperature"}},
    {"CCX_HYPERELASTIC_POLYNOMIAL_N1", "Hyperelasticity", "Polynomial N=1", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", {"C10","C01","D1","Temperature"}},
    {"CCX_HYPERELASTIC_POLYNOMIAL_N2", "Hyperelasticity", "Polynomial N=2", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", {"C10","C01","C20","C11","C02","D1","D2","Temperature"}},
    {"CCX_HYPERELASTIC_POLYNOMIAL_N3", "Hyperelasticity", "Polynomial N=3", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", {"C10","C01","C20","C11","C02","C30","C21","C12","C03","D1","D2","D3","Temperature"}},
    {"CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1", "Hyperelasticity", "Reduced Polynomial N=1", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", {"C10","D1","Temperature"}},
    {"CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2", "Hyperelasticity", "Reduced Polynomial N=2", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", {"C10","C20","D1","D2","Temperature"}},
    {"CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3", "Hyperelasticity", "Reduced Polynomial N=3", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", {"C10","C20","C30","D1","D2","D3","Temperature"}},
    {"CCX_HYPERELASTIC_YEOH", "Hyperelasticity", "Yeoh", "CCX_HYPERELASTIC_USE_CARD", "CCX_HYPERELASTIC_YEOH_USE_CARD", {"C10","C20","C30","D1","D2","D3","Temperature"}},
    {"CCX_HYPERFOAM_HYPERFOAM_N1", "Hyperfoam", "N1", "CCX_HYPERFOAM_USE_CARD", "CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD", {u8"\u03BC1",u8"\u03B11",u8"\u03BD1","Temperature"}},
    {"CCX_HYPERFOAM_HYPERFOAM_N2", "Hyperfoam", "N2", "CCX_HYPERFOAM_USE_CARD", "CCX_HYPERFOAM_HYPERFOAM_N2_USE_CARD", {u8"\u03BC1",u8"\u03B11",u8"\u03BC2",u8"\u03B12",u8"\u03BD1",u8"\u03BD2","Temperature"}},
    {"CCX_HYPERFOAM_HYPERFOAM_N3", "Hyperfoam", "N3", "CCX_HYPERFOAM_USE_CARD", "CCX_HYPERFOAM_HYPERFOAM_N3_USE_CARD", {u8"\u03BC1",u8"\u03B11",u8"\u03BC2",u8"\u03B12",u8"\u03BC3",u8"\u03B13",u8"\u03BD1",u8"\u03BD2",u8"\u03BD3","Temperature"}},
    {"CCX_COMPRESSION_ONLY_COMPRESSION_ONLY", "Compression only", "", "CCX_COMPRESSION_ONLY_USE_CARD", "", {"E","maximum allowed value","Temperature"}},
    {"CCX_TENSION_ONLY_TENSION_ONLY", "Tension only", "", "CCX_TENSION_ONLY_USE_CARD", "", {"E","maximum allowed value","Temperature"}},
    {"CCX_EXPANSION_ZERO", "Expansion", "", "CCX_EXPANSION_USE_CARD", "CCX_EXPANSION_ZERO_USE_CARD", {"Reference temperature"}},
    {"CCX_DAMPING_DAMPING", "Damping", "", "CCX_DAMPING_USE_CARD", "", {"Structural damping"}},
  };
  return cards;
}

bool switchEnabled(const MaterialProperties& material, const std::string& name)
{
  if (name.empty())
  {
    return true;
  }
  for (const auto& property : material.properties)
  {
    if (property.name == name)
    {
      return !property.values.empty() && !property.values.front().empty() && property.values.front().front() == 1;
    }
  }
  return false;
}
}

const MaterialCardDefinition* MaterialCardPresentation::find(const std::string& property)
{
  const auto& cards = definitions();
  const auto found = std::find_if(cards.begin(), cards.end(), [&](const auto& card) { return card.property == property; });
  return found == cards.end() ? nullptr : &*found;
}

QStringList MaterialCardPresentation::columns(const std::string& property)
{
  const auto* card = find(property);
  return card == nullptr ? QStringList{} : card->columns;
}

bool MaterialCardPresentation::enabled(const MaterialCardDefinition& definition, const MaterialProperties& material)
{
  return switchEnabled(material, definition.parentSwitch) && switchEnabled(material, definition.subtypeSwitch);
}
