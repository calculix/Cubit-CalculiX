#include "MaterialDefinitions.hpp"
#include <algorithm>
#include <cmath>
#include <cctype>
namespace {
std::string normalized(std::string value) {
  for (char& character : value) {
    character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    if (character == ' ' || character == '-') {
      character = '_';
    }
  }
  return value;
}
}
MaterialDefinitions::MaterialDefinitions() {
  cardDefinitions.push_back({"elastic", "Elasticity", "CCX_ELASTIC_", {}});
  cardDefinitions.back().models.push_back({"iso", "Isotropic", "CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE", "CCX_ELASTIC_ISO_USE_CARD", {{"youngs_modulus", "Young's Modulus", "stress"}, {"poisson_ratio", "Poisson's ratio", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ortho", "Orthotropic", "CCX_ELASTIC_ORTHO_CONSTANTS_VS_TEMPERATURE", "CCX_ELASTIC_ORTHO_USE_CARD", {{"d1111", "D1111", "stress"}, {"d1122", "D1122", "stress"}, {"d2222", "D2222", "stress"}, {"d1133", "D1133", "stress"}, {"d2233", "D2233", "stress"}, {"d3333", "D3333", "stress"}, {"d1212", "D1212", "stress"}, {"d1313", "D1313", "stress"}, {"d2323", "D2323", "stress"}, {"temperature", "TEMPERATURE", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ec", "Engineering constants", "CCX_ELASTIC_EC_CONSTANTS_VS_TEMPERATURE", "CCX_ELASTIC_EC_USE_CARD", {{"e1", "E1", "stress"}, {"e2", "E2", "stress"}, {"e3", "E3", "stress"}, {"v12", "v12", "dimensionless"}, {"v13", "v13", "dimensionless"}, {"v23", "v23", "dimensionless"}, {"g12", "G12", "stress"}, {"g13", "G13", "stress"}, {"g23", "G23", "stress"}, {"temperature", "TEMPERATURE", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"aniso", "Anisotropic", "CCX_ELASTIC_ANISO_CONSTANTS_VS_TEMPERATURE", "CCX_ELASTIC_ANISO_USE_CARD", {{"d1111", "D1111", "stress"}, {"d1122", "D1122", "stress"}, {"d2222", "D2222", "stress"}, {"d1133", "D1133", "stress"}, {"d2233", "D2233", "stress"}, {"d3333", "D3333", "stress"}, {"d1112", "D1112", "stress"}, {"d2212", "D2212", "stress"}, {"d3312", "D3312", "stress"}, {"d1212", "D1212", "stress"}, {"d1113", "D1113", "stress"}, {"d2213", "D2213", "stress"}, {"d3313", "D3313", "stress"}, {"d1213", "D1213", "stress"}, {"d1313", "D1313", "stress"}, {"d1123", "D1123", "stress"}, {"d2223", "D2223", "stress"}, {"d3323", "D3323", "stress"}, {"d1223", "D1223", "stress"}, {"d1323", "D1323", "stress"}, {"d2323", "D2323", "stress"}, {"temperature", "TEMPERATURE", "temperature", true}}, false});
  cardDefinitions.push_back({"plastic", "Plasticity", "CCX_PLASTIC_", {}});
  cardDefinitions.back().models.push_back({"iso", "Isotropic", "CCX_PLASTIC_ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "CCX_PLASTIC_ISO_USE_CARD", {{"yield_stress", "von Mises Stress", "stress"}, {"plastic_strain", "Plastic Strain", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"kin", "Kinematic", "CCX_PLASTIC_KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "CCX_PLASTIC_KIN_USE_CARD", {{"yield_stress", "von Mises Stress", "stress"}, {"plastic_strain", "Plastic Strain", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"combined", "Combined", "CCX_PLASTIC_COMBINED_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "CCX_PLASTIC_COMBINED_USE_CARD", {{"yield_stress", "von Mises Stress", "stress"}, {"plastic_strain", "Plastic Strain", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"johnson_cook", "Johnson-Cook", "CCX_PLASTIC_JOHNSON_COOK", "CCX_PLASTIC_JOHNSON_COOK_USE_CARD", {{"a", "A", "stress"}, {"b", "B", "stress"}, {"n", "n", "dimensionless"}, {"m", "m", "dimensionless"}, {"tm", "Tm", "temperature"}, {"t0", "T0", "temperature"}}, false});
  cardDefinitions.push_back({"density", "Density", "CCX_DENSITY_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_DENSITY_DENSITY", "", {{"density", "Density", "density"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"specific_heat", "Specific heat", "CCX_SPECIFIC_HEAT_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_SPECIFIC_HEAT_SPECIFIC_HEAT", "", {{"specific_heat", "Specific heat", "specific_heat"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"expansion", "Expansion", "CCX_EXPANSION_", {}});
  cardDefinitions.back().models.push_back({"iso", "Isotropic", "CCX_EXPANSION_ISO_A_TEMPERATURE", "CCX_EXPANSION_ISO_USE_CARD", {{"alpha", u8"\u03b1", "expansion"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ortho", "Orthotropic", "CCX_EXPANSION_ORTHO_CONSTANTS_VS_TEMPERATURE", "CCX_EXPANSION_ORTHO_USE_CARD", {{"alpha11", u8"\u03b111", "expansion"}, {"alpha22", u8"\u03b122", "expansion"}, {"alpha33", u8"\u03b133", "expansion"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"aniso", "Anisotropic", "CCX_EXPANSION_ANISO_CONSTANTS_VS_TEMPERATURE", "CCX_EXPANSION_ANISO_USE_CARD", {{"alpha11", u8"\u03b111", "expansion"}, {"alpha22", u8"\u03b122", "expansion"}, {"alpha33", u8"\u03b133", "expansion"}, {"alpha12", u8"\u03b112", "expansion"}, {"alpha13", u8"\u03b113", "expansion"}, {"alpha23", u8"\u03b123", "expansion"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"conductivity", "Conductivity", "CCX_CONDUCTIVITY_", {}});
  cardDefinitions.back().models.push_back({"iso", "Isotropic", "CCX_CONDUCTIVITY_ISO_K_TEMPERATURE", "CCX_CONDUCTIVITY_ISO_USE_CARD", {{"kappa", u8"\u03ba", "conductivity"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ortho", "Orthotropic", "CCX_CONDUCTIVITY_ORTHO_CONSTANTS_VS_TEMPERATURE", "CCX_CONDUCTIVITY_ORTHO_USE_CARD", {{"kappa11", u8"\u03ba11", "conductivity"}, {"kappa22", u8"\u03ba22", "conductivity"}, {"kappa33", u8"\u03ba33", "conductivity"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"aniso", "Anisotropic", "CCX_CONDUCTIVITY_ANISO_CONSTANTS_VS_TEMPERATURE", "CCX_CONDUCTIVITY_ANISO_USE_CARD", {{"kappa11", u8"\u03ba11", "conductivity"}, {"kappa22", u8"\u03ba22", "conductivity"}, {"kappa33", u8"\u03ba33", "conductivity"}, {"kappa12", u8"\u03ba12", "conductivity"}, {"kappa13", u8"\u03ba13", "conductivity"}, {"kappa23", u8"\u03ba23", "conductivity"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"deformation_plasticity", "Deformation plasticity", "CCX_DEFORMATION_PLASTICITY_", {}});
  cardDefinitions.back().models.push_back({"default", "Ramberg-Osgood", "CCX_DEFORMATION_PLASTICITY_RAMBERG_OSGOOD", "", {{"youngs_modulus", "Young's Modulus", "stress"}, {"poisson_ratio", "Poisson's ratio", "dimensionless"}, {"yield_stress", "Yield Stress", "stress"}, {"exponent", "Exponent", "dimensionless"}, {"yield_offset", "Yield Offset", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"mohr_coulomb", "Mohr-Coulomb", "CCX_MOHR_COULOMB_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_MOHR_COULOMB_MOHR_COULOMB", "", {{"friction_angle", "Friction angle", "angle"}, {"dilation_angle", "Dilation angle", "angle"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"mohr_coulomb_hardening", "Mohr-Coulomb hardening", "CCX_MOHR_COULOMB_HARDENING_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_MOHR_COULOMB_HARDENING_MOHR_COULOMB_HARDENING", "", {{"cohesion", "Cohesion yield stress c", "stress"}, {"plastic_strain", "Equivalent plastic strain", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"cyclic_hardening", "Cyclic hardening", "CCX_CYCLIC_HARDENING_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_CYCLIC_HARDENING_CYCLIC_HARDENING", "", {{"yield_stress", "Von Mises stress", "stress"}, {"plastic_strain", "Equivalent plastic strain", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"rate_dependent", "Rate dependence", "CCX_RATE_DEPENDENT_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_RATE_DEPENDENT_RATE_DEPENDENT", "", {{"c", "C", "dimensionless"}, {"epsilon_dot_0", u8"\u03b5_dot_0", "inverse_time"}}, false});
  cardDefinitions.push_back({"creep", "Creep", "CCX_CREEP_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_CREEP_CREEP", "", {{"a", "A", "creep_coefficient"}, {"n", "n", "dimensionless"}, {"m", "m", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"hyperelastic", "Hyperelasticity", "CCX_HYPERELASTIC_", {}});
  cardDefinitions.back().models.push_back({"arruda_boyce", "Arruda Boyce", "CCX_HYPERELASTIC_ARRUDA_BOYCE", "CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", {{"mu", u8"\u03bc", "stress"}, {"lambdam", u8"\u03bbm", "dimensionless"}, {"d", "D", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"mooney_rivlin", "Mooney Rivlin", "CCX_HYPERELASTIC_MOONEY_RIVLIN", "CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", {{"c10", "C10", "stress"}, {"c01", "C01", "stress"}, {"d1", "D1", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"neo_hooke", "Neo Hooke", "CCX_HYPERELASTIC_NEO_HOOKE", "CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", {{"c10", "C10", "stress"}, {"d1", "D1", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ogden_n1", "Ogden N=1", "CCX_HYPERELASTIC_OGDEN_N1", "CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", {{"mu1", u8"\u03bc1", "stress"}, {"alpha1", u8"\u03b11", "dimensionless"}, {"d1", "D1", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ogden_n2", "Ogden N=2", "CCX_HYPERELASTIC_OGDEN_N2", "CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", {{"mu1", u8"\u03bc1", "stress"}, {"alpha1", u8"\u03b11", "dimensionless"}, {"d1", "D1", "inverse_stress"}, {"mu2", u8"\u03bc2", "stress"}, {"alpha2", u8"\u03b12", "dimensionless"}, {"d2", "D2", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"ogden_n3", "Ogden N=3", "CCX_HYPERELASTIC_OGDEN_N3", "CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", {{"mu1", u8"\u03bc1", "stress"}, {"alpha1", u8"\u03b11", "dimensionless"}, {"d1", "D1", "inverse_stress"}, {"mu2", u8"\u03bc2", "stress"}, {"alpha2", u8"\u03b12", "dimensionless"}, {"d2", "D2", "inverse_stress"}, {"mu3", u8"\u03bc3", "stress"}, {"alpha3", u8"\u03b13", "dimensionless"}, {"d3", "D3", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"polynomial_n1", "Polynomial N=1", "CCX_HYPERELASTIC_POLYNOMIAL_N1", "CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", {{"c10", "C10", "stress"}, {"c01", "C01", "stress"}, {"d1", "D1", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"polynomial_n2", "Polynomial N=2", "CCX_HYPERELASTIC_POLYNOMIAL_N2", "CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", {{"c10", "C10", "stress"}, {"c01", "C01", "stress"}, {"c20", "C20", "stress"}, {"c11", "C11", "stress"}, {"c02", "C02", "stress"}, {"d1", "D1", "inverse_stress"}, {"d2", "D2", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"polynomial_n3", "Polynomial N=3", "CCX_HYPERELASTIC_POLYNOMIAL_N3", "CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", {{"c10", "C10", "stress"}, {"c01", "C01", "stress"}, {"c20", "C20", "stress"}, {"c11", "C11", "stress"}, {"c02", "C02", "stress"}, {"c30", "C30", "stress"}, {"c21", "C21", "stress"}, {"c12", "C12", "stress"}, {"c03", "C03", "stress"}, {"d1", "D1", "inverse_stress"}, {"d2", "D2", "inverse_stress"}, {"d3", "D3", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"reduced_polynomial_n1", "Reduced Polynomial N=1", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", {{"c10", "C10", "stress"}, {"d1", "D1", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"reduced_polynomial_n2", "Reduced Polynomial N=2", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", {{"c10", "C10", "stress"}, {"c20", "C20", "stress"}, {"d1", "D1", "inverse_stress"}, {"d2", "D2", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"reduced_polynomial_n3", "Reduced Polynomial N=3", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3", "CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", {{"c10", "C10", "stress"}, {"c20", "C20", "stress"}, {"c30", "C30", "stress"}, {"d1", "D1", "inverse_stress"}, {"d2", "D2", "inverse_stress"}, {"d3", "D3", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"yeoh", "Yeoh", "CCX_HYPERELASTIC_YEOH", "CCX_HYPERELASTIC_YEOH_USE_CARD", {{"c10", "C10", "stress"}, {"c20", "C20", "stress"}, {"c30", "C30", "stress"}, {"d1", "D1", "inverse_stress"}, {"d2", "D2", "inverse_stress"}, {"d3", "D3", "inverse_stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"hyperfoam", "Hyperfoam", "CCX_HYPERFOAM_", {}});
  cardDefinitions.back().models.push_back({"hyperfoam_n1", "N1", "CCX_HYPERFOAM_HYPERFOAM_N1", "CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD", {{"mu1", u8"\u03bc1", "stress"}, {"alpha1", u8"\u03b11", "dimensionless"}, {"nu1", u8"\u03bd1", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"hyperfoam_n2", "N2", "CCX_HYPERFOAM_HYPERFOAM_N2", "CCX_HYPERFOAM_HYPERFOAM_N2_USE_CARD", {{"mu1", u8"\u03bc1", "stress"}, {"alpha1", u8"\u03b11", "dimensionless"}, {"mu2", u8"\u03bc2", "stress"}, {"alpha2", u8"\u03b12", "dimensionless"}, {"nu1", u8"\u03bd1", "dimensionless"}, {"nu2", u8"\u03bd2", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.back().models.push_back({"hyperfoam_n3", "N3", "CCX_HYPERFOAM_HYPERFOAM_N3", "CCX_HYPERFOAM_HYPERFOAM_N3_USE_CARD", {{"mu1", u8"\u03bc1", "stress"}, {"alpha1", u8"\u03b11", "dimensionless"}, {"mu2", u8"\u03bc2", "stress"}, {"alpha2", u8"\u03b12", "dimensionless"}, {"mu3", u8"\u03bc3", "stress"}, {"alpha3", u8"\u03b13", "dimensionless"}, {"nu1", u8"\u03bd1", "dimensionless"}, {"nu2", u8"\u03bd2", "dimensionless"}, {"nu3", u8"\u03bd3", "dimensionless"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"compression_only", "Compression only", "CCX_COMPRESSION_ONLY_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_COMPRESSION_ONLY_COMPRESSION_ONLY", "", {{"e", "E", "stress"}, {"maximum_stress", "maximum allowed value", "stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"tension_only", "Tension only", "CCX_TENSION_ONLY_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_TENSION_ONLY_TENSION_ONLY", "", {{"e", "E", "stress"}, {"maximum_stress", "maximum allowed value", "stress"}, {"temperature", "Temperature", "temperature", true}}, false});
  cardDefinitions.push_back({"damping", "Damping", "CCX_DAMPING_", {}});
  cardDefinitions.back().models.push_back({"default", "", "CCX_DAMPING_DAMPING", "", {{"structural_damping", "Structural damping", "dimensionless"}}, true});
}
MaterialDefinitions::~MaterialDefinitions() = default;
const std::vector<FrameworkMaterialCardDefinition>& MaterialDefinitions::cards() const noexcept {
  return cardDefinitions;
}
const FrameworkMaterialCardDefinition* MaterialDefinitions::card(const std::string& key) const noexcept {
  const std::string value = normalized(key);
  for (const auto& item : cardDefinitions) {
    if (item.key == value || normalized(item.prefix) == value || normalized(item.title) == value) {
      return &item;
    }
  }
  return nullptr;
}
const MaterialModelDefinition* MaterialDefinitions::model(const FrameworkMaterialCardDefinition& definition, const std::string& key) const noexcept {
  std::string value = normalized(key);
  if (value == "isotropic") {
    value = "iso";
  } else if (value == "orthotropic") {
    value = "ortho";
  } else if (value == "engineering_constants") {
    value = "ec";
  } else if (value == "anisotropic") {
    value = "aniso";
  } else if (value == "kinematic") {
    value = "kin";
  }
  for (const auto& item : definition.models) {
    if (item.key == value || normalized(item.title) == value || item.property == key || item.selectionProperty == key) {
      return &item;
    }
  }
  return nullptr;
}
const MaterialModelDefinition* MaterialDefinitions::property(const std::string& key, const FrameworkMaterialCardDefinition*& definition) const noexcept {
  definition = nullptr;
  for (const auto& item : cardDefinitions) {
    for (const auto& candidate : item.models) {
      if (candidate.property == key || candidate.selectionProperty == key) {
        definition = &item;
        return &candidate;
      }
    }
  }
  return nullptr;
}
std::vector<std::vector<std::string>> MaterialDefinitions::legacy_properties() const {
  std::vector<std::vector<std::string>> result;
  for (const auto& definition : cardDefinitions) {
    result.push_back({definition.prefix + "USE_CARD", "1"});
    for (const auto& item : definition.models) {
      if (!item.selectionProperty.empty()) {
        result.push_back({item.selectionProperty, "1"});
      }
      if (item.scalar) {
        result.push_back({item.property, "1"});
      } else {
        result.push_back({item.property, "4", std::to_string(item.columns.size())});
      }
    }
    if (definition.key == "expansion") {
      result.push_back({"CCX_EXPANSION_ZERO_USE_CARD", "1"});
      result.push_back({"CCX_EXPANSION_ZERO", "1"});
    }
  }
  return result;
}
bool MaterialDefinitions::validate(const MaterialData& data, bool requireActiveValues, std::string& error) const {
  if (data.schema != "calculix" || data.name.empty() || data.name.size() > 255) {
    error = "Material schema or name is invalid.";
    return false;
  }
  for (unsigned char character : data.name) {
    if (character < 32 || character == 127 || character == ',' || character == '"' || character == '/' || character == '\\') {
      error = "Material name contains an unsupported delimiter or control character.";
      return false;
    }
  }
  for (const auto& entry : data.cards) {
    const auto* definition = card(entry.first);
    if (definition == nullptr) {
      error = "Unknown material card: " + entry.first;
      return false;
    }
    const MaterialCard& cardData = entry.second;
    const auto* selected = model(*definition, cardData.model);
    if (selected == nullptr) {
      error = "Unknown material model: " + cardData.model;
      return false;
    }
    if (cardData.values.rows.size() > 1000000) {
      error = "Material table exceeds the supported row count.";
      return false;
    }
    if (selected->scalar && !cardData.values.rows.empty()) {
      error = "Scalar material card contains table rows.";
      return false;
    }
    if (!selected->scalar && cardData.values.scalar.has_value()) {
      error = "Tabular material card contains an unexpected scalar.";
      return false;
    }
    for (const auto& row : cardData.values.rows) {
      bool validWidth = row.values.size() == selected->columns.size();
      if (!selected->columns.empty() && selected->columns.back().optional && row.values.size() + 1 == selected->columns.size()) {
        validWidth = true;
      }
      if (!validWidth) {
        error = "Material table has an incorrect column count: " + selected->property;
        return false;
      }
      for (double value : row.values) {
        if (!std::isfinite(value)) {
          error = "Material values must be finite.";
          return false;
        }
      }
    }
    if (cardData.values.scalar.has_value() && !std::isfinite(*cardData.values.scalar)) {
      error = "Material scalar must be finite.";
      return false;
    }
    if (cardData.values.referenceTemperature.has_value() && !std::isfinite(*cardData.values.referenceTemperature)) {
      error = "Material reference temperature must be finite.";
      return false;
    }
    if (definition->key != "expansion" && cardData.values.referenceTemperature.has_value()) {
      error = "Only thermal expansion accepts a reference temperature.";
      return false;
    }
    if (requireActiveValues && cardData.state == MaterialCardState::Active) {
      if (selected->scalar && !cardData.values.scalar.has_value()) {
        error = "Active material card requires a scalar: " + definition->title;
        return false;
      }
      if (!selected->scalar && cardData.values.rows.empty()) {
        error = "Active material card requires table values: " + definition->title;
        return false;
      }
    }
  }
  return true;
}
