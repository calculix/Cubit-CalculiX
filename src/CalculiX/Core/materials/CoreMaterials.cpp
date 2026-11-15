#include "Core/materials/CoreMaterials.hpp"
#include "CoreMaterialsTopologyGrouping.hpp"
#include <string>
#include <vector>
#include <map>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/export/CalculiXExportInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "ScientificNumber.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
using MaterialVector = std::vector<double>;
using MaterialMatrix = std::vector<std::vector<double>>;
#include "CubitMessage.hpp"
#include <algorithm>
#include <set>
namespace {
std::map<std::string, std::string> material_keywords() {
  std::map<std::string, std::string> values;
  values.emplace("CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE", "*ELASTIC,TYPE=ISO");
  values.emplace("CCX_ELASTIC_ORTHO_CONSTANTS_VS_TEMPERATURE", "*ELASTIC,TYPE=ORTHO");
  values.emplace("CCX_ELASTIC_EC_CONSTANTS_VS_TEMPERATURE", "*ELASTIC,TYPE=ENGINEERING CONSTANTS");
  values.emplace("CCX_ELASTIC_ANISO_CONSTANTS_VS_TEMPERATURE", "*ELASTIC,TYPE=ANISO");
  values.emplace("CCX_PLASTIC_ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "*PLASTIC,HARDENING=ISOTROPIC");
  values.emplace("CCX_PLASTIC_KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "*PLASTIC,HARDENING=KINEMATIC");
  values.emplace("CCX_PLASTIC_COMBINED_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE", "*PLASTIC,HARDENING=COMBINED");
  values.emplace("CCX_PLASTIC_JOHNSON_COOK", "*PLASTIC,HARDENING=JOHNSON COOK");
  values.emplace("CCX_DENSITY_DENSITY", "*DENSITY");
  values.emplace("CCX_SPECIFIC_HEAT_SPECIFIC_HEAT", "*SPECIFIC HEAT");
  values.emplace("CCX_EXPANSION_ISO_A_TEMPERATURE", "*EXPANSION,TYPE=ISO");
  values.emplace("CCX_EXPANSION_ORTHO_CONSTANTS_VS_TEMPERATURE", "*EXPANSION,TYPE=ORTHO");
  values.emplace("CCX_EXPANSION_ANISO_CONSTANTS_VS_TEMPERATURE", "*EXPANSION,TYPE=ANISO");
  values.emplace("CCX_CONDUCTIVITY_ISO_K_TEMPERATURE", "*CONDUCTIVITY,TYPE=ISO");
  values.emplace("CCX_CONDUCTIVITY_ORTHO_CONSTANTS_VS_TEMPERATURE", "*CONDUCTIVITY,TYPE=ORTHO");
  values.emplace("CCX_CONDUCTIVITY_ANISO_CONSTANTS_VS_TEMPERATURE", "*CONDUCTIVITY,TYPE=ANISO");
  values.emplace("CCX_DEFORMATION_PLASTICITY_RAMBERG_OSGOOD", "*DEFORMATION PLASTICITY");
  values.emplace("CCX_MOHR_COULOMB_MOHR_COULOMB", "*MOHR COULOMB");
  values.emplace("CCX_MOHR_COULOMB_HARDENING_MOHR_COULOMB_HARDENING", "*MOHR COULOMB HARDENING");
  values.emplace("CCX_CYCLIC_HARDENING_CYCLIC_HARDENING", "*CYCLIC HARDENING");
  values.emplace("CCX_RATE_DEPENDENT_RATE_DEPENDENT", "*RATE DEPENDENT,TYPE=JOHNSON COOK");
  values.emplace("CCX_CREEP_CREEP", "*CREEP,LAW=NORTON");
  values.emplace("CCX_HYPERELASTIC_ARRUDA_BOYCE", "*HYPERELASTIC,ARRUDA-BOYCE");
  values.emplace("CCX_HYPERELASTIC_MOONEY_RIVLIN", "*HYPERELASTIC,MOONEY-RIVLIN");
  values.emplace("CCX_HYPERELASTIC_NEO_HOOKE", "*HYPERELASTIC,NEO HOOKE");
  values.emplace("CCX_HYPERELASTIC_OGDEN_N1", "*HYPERELASTIC,OGDEN,N=1");
  values.emplace("CCX_HYPERELASTIC_OGDEN_N2", "*HYPERELASTIC,OGDEN,N=2");
  values.emplace("CCX_HYPERELASTIC_OGDEN_N3", "*HYPERELASTIC,OGDEN,N=3");
  values.emplace("CCX_HYPERELASTIC_POLYNOMIAL_N1", "*HYPERELASTIC,POLYNOMIAL,N=1");
  values.emplace("CCX_HYPERELASTIC_POLYNOMIAL_N2", "*HYPERELASTIC,POLYNOMIAL,N=2");
  values.emplace("CCX_HYPERELASTIC_POLYNOMIAL_N3", "*HYPERELASTIC,POLYNOMIAL,N=3");
  values.emplace("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1", "*HYPERELASTIC,REDUCED_POLYNOMIAL,N=1");
  values.emplace("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2", "*HYPERELASTIC,REDUCED_POLYNOMIAL,N=2");
  values.emplace("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3", "*HYPERELASTIC,REDUCED_POLYNOMIAL,N=3");
  values.emplace("CCX_HYPERELASTIC_YEOH", "*HYPERELASTIC,YEOH");
  values.emplace("CCX_HYPERFOAM_HYPERFOAM_N1", "*HYPERFOAM,N=1");
  values.emplace("CCX_HYPERFOAM_HYPERFOAM_N2", "*HYPERFOAM,N=2");
  values.emplace("CCX_HYPERFOAM_HYPERFOAM_N3", "*HYPERFOAM,N=3");
  values.emplace("CCX_COMPRESSION_ONLY_COMPRESSION_ONLY", "*USER MATERIAL,CONSTANTS=2");
  values.emplace("CCX_TENSION_ONLY_TENSION_ONLY", "*USER MATERIAL,CONSTANTS=2");
  values.emplace("CCX_DAMPING_DAMPING", "*DAMPING");
  return values;
}
}
CoreMaterials::CoreMaterials() : material_api("calculix", MaterialValueSpace::Solver) {
  mat_iface = &material_api;
  is_initialized = true;
}

CoreMaterials::~CoreMaterials() {}

bool CoreMaterials::update() {
    return true;
}

bool CoreMaterials::reset() {
  topology_material_data.clear();
  material_api.reset();
  return true;
}

bool CoreMaterials::check_initialized() {
  return is_initialized;
}

std::vector<std::string> CoreMaterials::get_group_list() { // get a list of the CalculiX Material Groups
  std::vector<std::string> groups;
  groups.push_back("SIM_CCX");
  return groups;
}

int CoreMaterials::get_group_property_size(std::string group_property) {
  return material_api.get_property_size(group_property);
}

std::string CoreMaterials::get_material_export() { // get a list of the CalculiX Material exports
  std::string error;
  if (!material_api.validate_materials(error)) {
    PRINT_ERROR("%s\n", error.c_str());
    return {};
  }
  CalculiXExportInterface ccx_export_iface;

  std::vector<std::string> materials_export_list;
  materials_export_list.push_back("********************************** M A T E R I A L S ****************************");

  std::vector<std::string> material_name_list;
  for (int id : material_api.get_material_ids()) {
    material_name_list.push_back(material_api.get_material_name(id));
  }
  //loop over all materials
  for (size_t i = 0; i < material_name_list.size(); i++) {
    FrameworkMaterialsInterface::Material material;
    FrameworkMaterialsInterface::PropertyGroup group;
    std::string group_name;
    std::vector<std::string> group_list;
    material = mat_iface->get_material(material_name_list[i]);
    group = mat_iface->get_material_property_group(material);
    group_name = mat_iface->get_group_name(group);
    group_list = this->get_group_list();

    if (std::find(group_list.begin(), group_list.end(), group_name) != group_list.end()) {
      // CUSTOMLINE START
      std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEFORE","MATERIAL",mat_iface->get_material_id(material));
      for (size_t icl = 0; icl < customline.size(); icl++) {
        materials_export_list.push_back(customline[icl]);
      }
      // CUSTOMLINE END

      //check if name needs to be changed, like for compression only
      std::string prefix = this->get_material_prefix(material_name_list[i]);

      materials_export_list.push_back("** CUBIT_SIM_MATERIAL_ID=" + std::to_string(material) + ";" + prefix + material_name_list[i]);
      materials_export_list.push_back("*MATERIAL, NAME=" + prefix + material_name_list[i]); //material_name
      const std::string cards = get_material_cards_export(material_name_list[i], group_name);
      bool active = false;
      const auto* instance = material_api.find(material);
      if (instance != nullptr) {
        for (const auto& entry : instance->data.cards) {
          if (entry.second.state == MaterialCardState::Active) {
            active = true;
          }
        }
      }
      if (active && cards.empty()) {
        return {};
      }
      materials_export_list.push_back(cards);

      // CUSTOMLINE START
      customline = ccx_export_iface.get_customline_data("AFTER","MATERIAL",mat_iface->get_material_id(material));
      for (size_t icl = 0; icl < customline.size(); icl++) {
        materials_export_list.push_back(customline[icl]);
      }
      // CUSTOMLINE END
    }
  }

  std::string material_export;

  for (size_t i = 0; i < materials_export_list.size(); i++) {
    material_export.append(materials_export_list[i] + "\n");
  }

  return material_export;
}

std::string CoreMaterials::get_material_cards_export(std::string material_name, std::string group_name) {
  static_cast<void>(group_name);
  const int id = material_api.get_material(material_name);
  const auto* material = material_api.find(id);
  if (material == nullptr) {
    PRINT_ERROR("Material %s does not exist in the CalculiX workspace.\n", material_name.c_str());
    return {};
  }
  std::string error;
  if (!framework_materials().definitions().validate(material->data, true, error)) {
    PRINT_ERROR("%s: %s\n", material_name.c_str(), error.c_str());
    return {};
  }
  static const std::map<std::string, std::string> keywords = material_keywords();
  CalculiXApplicationInterface application;
  std::string output;
  for (const auto& definition : framework_materials().definitions().cards()) {
    auto found = material->data.cards.find(definition.key);
    if (found == material->data.cards.end() || found->second.state == MaterialCardState::Inactive) {
      continue;
    }
    const auto* model = framework_materials().definitions().model(definition, found->second.model);
    if (model == nullptr || keywords.count(model->property) == 0) {
      PRINT_ERROR("Material model has no CalculiX exporter.\n");
      return {};
    }
    std::string keyword = keywords.at(model->property);
    if (model->scalar) {
      double value = 0.0;
      if (!material_api.read_scalar(material->data, model->property, value, error)) {
        PRINT_ERROR("%s\n", error.c_str());
        return {};
      }
      output += keyword + ",STRUCTURAL=" + application.to_string_scientific(value) + "\n";
      continue;
    }
    if (definition.key == "expansion" && found->second.values.referenceTemperature.has_value()) {
      double reference = 0.0;
      if (!material_api.read_scalar(material->data, "CCX_EXPANSION_ZERO", reference, error)) {
        PRINT_ERROR("%s\n", error.c_str());
        return {};
      }
      keyword += ",ZERO=" + application.to_string_scientific(reference);
    }
    output += keyword + "\n";
    std::vector<std::vector<double>> rows;
    if (!material_api.read_table(material->data, model->property, rows, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      return {};
    }
    bool wrapEight = false;
    if (definition.key == "elastic" && model->key != "iso") {
      wrapEight = true;
    } else if (definition.key == "hyperelastic" && (model->key == "ogden_n3" || model->key == "polynomial_n3")) {
      wrapEight = true;
    }
    for (const auto& row : rows) {
      for (std::size_t column = 0; column < model->columns.size(); ++column) {
        if (column < row.size()) {
          output += application.to_string_scientific(row[column]);
        }
        if (column + 1 == model->columns.size()) {
          output += "\n";
        } else if (wrapEight && (column + 1) % 8 == 0) {
          output += ",\n";
        } else {
          output += ",";
        }
      }
    }
  }
  return output;
}

std::string CoreMaterials::get_material_prefix(std::string material_name) {
  const auto* material = material_api.find(material_api.get_material(material_name));
  if (material == nullptr) {
    return {};
  }
  auto compression = material->data.cards.find("compression_only");
  if (compression != material->data.cards.end() && compression->second.state == MaterialCardState::Active) {
    return "COMPRESSION_ONLY_";
  }
  auto tension = material->data.cards.find("tension_only");
  if (tension != material->data.cards.end() && tension->second.state == MaterialCardState::Active) {
    return "TENSION_ONLY_";
  }
  auto plastic = material->data.cards.find("plastic");
  if (plastic != material->data.cards.end() && plastic->second.state == MaterialCardState::Active && plastic->second.model == "johnson_cook") {
    return "JOHNSONCOOK_";
  }
  return {};
}

bool CoreMaterials::check_material_exists(int material_id) {
  FrameworkMaterialsInterface::Material material;
  material = mat_iface->get_material(material_id);
  if (material != NULL) {
    return true;
  }
  return false;
}

bool CoreMaterials::initialize_topology_materials(int topology_id,const std::vector<int>& element_ids,const std::vector<int>& base_material_ids) {
  CalculiXMeshInterface ccx_mesh_iface;

  if (!is_initialized || !mat_iface || topology_id <= 0 ||
      element_ids.empty() ||
      element_ids.size() != base_material_ids.size() ||
      topology_material_data.count(topology_id) != 0) {
    return false;
  }

  TopologyMaterialData collection;

  for (size_t i = 0; i < element_ids.size(); ++i) {
    const int element_id = element_ids[i];
    const int material_id = base_material_ids[i];

    if (!ccx_mesh_iface.check_global_element_exists(element_id)) {
      return false;
    }

    if (!collection.element_base_material.emplace(
            element_id, material_id).second) {
      return false;
    } // Duplicate element.

    // Read each distinct base material only once.
    if (collection.base_materials.count(material_id) != 0) {
      continue;
    }

    const auto* material = material_api.find(material_id);
    if (material == nullptr) {
      return false;
    }
    auto elastic = material->data.cards.find("elastic");
    if (elastic == material->data.cards.end() || elastic->second.state != MaterialCardState::Active || elastic->second.model != "iso") {
      return false;
    }
    auto plastic = material->data.cards.find("plastic");
    if (plastic != material->data.cards.end() && plastic->second.state == MaterialCardState::Active) {
      return false;
    }
    const auto& rows = elastic->second.values.rows;
    if (rows.size() != 1 || (rows.front().values.size() != 2 && rows.front().values.size() != 3)) {
      return false;
    }
    const auto& properties = rows.front().values;
    const double youngs_modulus = properties[0];
    const double poissons_ratio = properties[1];

    if (!std::isfinite(youngs_modulus) ||
        !std::isfinite(poissons_ratio) ||
        youngs_modulus <= 0.0 ||
        poissons_ratio <= -1.0 ||
        poissons_ratio >= 0.5) {
      return false;
    }

    collection.base_materials.emplace(material_id, TopologyElasticMaterial{youngs_modulus, poissons_ratio});
  }

  topology_material_data.emplace(topology_id, std::move(collection));
  return true;
}

bool CoreMaterials::set_topology_iteration_factors(int topology_id,int iteration_id,const std::vector<int>& element_ids,const std::vector<double>& stiffness_factors, int material_cap, bool stable_levels, double minimum_factor) {
  auto collection = topology_material_data.find(topology_id);

  if (collection == topology_material_data.end() || iteration_id < 1 || element_ids.empty() || element_ids.size() != stiffness_factors.size() || material_cap < 2) {
    PRINT_ERROR("Invalid topology iteration material data.\n");
    return false;
  }

  auto& data = collection->second;

  if (data.iteration_factors.count(iteration_id) != 0 || data.iteration_material_caps.count(iteration_id) != 0 || data.iteration_material_groups.count(iteration_id) != 0 || element_ids.size() != data.element_base_material.size()) {
    PRINT_ERROR("Topology iteration materials already exist or element coverage is incomplete.\n");
    return false;
  }

  std::map<int, double> factors;

  for (std::size_t i = 0; i < element_ids.size(); ++i) {
    const int element_id = element_ids[i];
    const double factor = stiffness_factors[i];

    if (data.element_base_material.count(element_id) == 0 || !std::isfinite(factor) || factor <= 0.0 || factor > 1.0 || !factors.emplace(element_id, factor).second) {
      PRINT_ERROR("Invalid or duplicate topology factor for element %d.\n", element_id);
      return false;
    }
  }

  TopologyMaterialGrouping grouping;
  std::map<int, double> applied_factors;

  if (stable_levels) {
    const std::map<int, double>* previous = nullptr;
    if (iteration_id > 1) {
      const auto old = data.iteration_factors.find(iteration_id - 1);
      const auto old_cap = data.iteration_material_caps.find(iteration_id - 1);
      if (old == data.iteration_factors.end() || old_cap == data.iteration_material_caps.end() || old_cap->second != material_cap) {
        PRINT_ERROR("Missing previous topology factors or changed material cap.\n");
        return false;
      }
      previous = &old->second;
    }
    std::map<int, double> stable_factors;
    if (!CoreMaterialsTopologyGrouping::apply(data.element_base_material, factors, previous, material_cap, minimum_factor, stable_factors) ||
        !build_topology_material_groups(data, stable_factors, material_cap, grouping, applied_factors, false)) {
      return false;
    }
    grouping.maximum_relative_factor_error = 0.0;
    for (const auto& requested : factors) {
      const double error = std::abs(applied_factors.at(requested.first) / requested.second - 1.0);
      grouping.maximum_relative_factor_error = (std::max)(grouping.maximum_relative_factor_error, error);
    }
  }
  else if (!build_topology_material_groups(data, factors, material_cap, grouping, applied_factors, true)) {
    return false;
  }

  const std::size_t group_count = grouping.groups.size();
  const double maximum_error = grouping.maximum_relative_factor_error;
  data.iteration_factors.emplace(iteration_id, std::move(applied_factors));
  data.iteration_material_caps.emplace(iteration_id, material_cap);
  // Retain only one element-to-group cache per topology; old iterations rebuild exactly.
  data.iteration_material_groups.clear();
  data.iteration_material_groups.emplace(iteration_id, std::move(grouping));

  PRINT_INFO("Topology %d iteration %d: %llu material groups, cap %d, maximum relative stiffness-factor quantization error %.6f%%.\n", topology_id, iteration_id, static_cast<unsigned long long>(group_count), material_cap, 100.0 * maximum_error);
  return true;
}

std::string CoreMaterials::get_topology_material_name(int topology_id,int element_id) const {
  return "CCX_TOPO_" + std::to_string(topology_id)
       + "_E" + std::to_string(element_id);
}

std::string CoreMaterials::get_topology_material_export_legacy(int topology_id,int iteration_id) {
  const auto collection_it = topology_material_data.find(topology_id);

  if (collection_it == topology_material_data.end()) {
    PRINT_ERROR("Topology material collection %d not found.\n",
                topology_id);
    return "";
  }

  const auto& collection = collection_it->second;
  const auto iteration_it =
      collection.iteration_factors.find(iteration_id);

  if (iteration_it == collection.iteration_factors.end()) {
    PRINT_ERROR("Iteration %d not found for topology %d.\n",
                iteration_id, topology_id);
    return "";
  }

  const auto& factors = iteration_it->second;

  if (factors.empty() ||
      factors.size() != collection.element_base_material.size()) {
    PRINT_ERROR("Topology %d iteration %d is incomplete.\n",
                topology_id, iteration_id);
    return "";
  }

  std::ostringstream output;
  output << std::scientific << std::setprecision(6);

  output << "** TOPOLOGY MATERIALS: RUN " << topology_id
         << ", ITERATION " << iteration_id << "\n";

  for (const auto& assignment : collection.element_base_material) {
    const int element_id = assignment.first;
    const int base_material_id = assignment.second;

    const auto material_it =
        collection.base_materials.find(base_material_id);
    const auto factor_it = factors.find(element_id);

    if (material_it == collection.base_materials.end() ||
        factor_it == factors.end()) {
      PRINT_ERROR("Missing topology material data for element %d.\n",
                  element_id);
      return "";
    }

    const auto& material = material_it->second;
    const double factor = factor_it->second;
    double modulus = 0.0;
    std::string conversionError;
    if (!framework_units().to_solver(material.youngs_modulus * factor, SimUnits::Stress, "calculix", modulus, conversionError)) {
      PRINT_ERROR("%s\n", conversionError.c_str());
      return {};
    }

    if (!std::isfinite(factor) || factor <= 0.0 || factor > 1.0 ||
        !std::isfinite(modulus) || modulus <= 0.0 ||
        !std::isfinite(material.poissons_ratio) ||
        material.poissons_ratio <= -1.0 ||
        material.poissons_ratio >= 0.5) {
      PRINT_ERROR("Invalid topology elastic properties for element %d.\n",
                  element_id);
      return "";
    }

    output << "** CUBIT_SIM_MATERIAL_ID=" << base_material_id << ";" << get_topology_material_name(topology_id, element_id) << "\n";
    output << "*MATERIAL, NAME="
           << get_topology_material_name(topology_id, element_id)
           << "\n"
           << "*ELASTIC\n"
           << modulus << ", " << material.poissons_ratio << "\n";
  }

  return output.str();
}

bool CoreMaterials::build_topology_material_groups(const TopologyMaterialData& data, const std::map<int, double>& requested_factors, int material_cap, TopologyMaterialGrouping& grouping, std::map<int, double>& applied_factors, bool quantize) const {
  if (material_cap < 2 || requested_factors.empty() || requested_factors.size() != data.element_base_material.size() || requested_factors.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
    PRINT_ERROR("Invalid topology material grouping request.\n");
    return false;
  }

  std::map<int, std::set<double>> distinct_factors;

  for (const auto& assignment : data.element_base_material) {
    const auto material = data.base_materials.find(assignment.second);
    const auto factor = requested_factors.find(assignment.first);

    if (assignment.first <= 0 || material == data.base_materials.end() || factor == requested_factors.end()) {
      PRINT_ERROR("Missing topology material or factor for element %d.\n", assignment.first);
      return false;
    }

    const auto& properties = material->second;

    if (!std::isfinite(factor->second) || factor->second <= 0.0 || factor->second > 1.0 || !std::isfinite(properties.youngs_modulus) || properties.youngs_modulus <= 0.0 || !std::isfinite(properties.poissons_ratio) || properties.poissons_ratio <= -1.0 || properties.poissons_ratio >= 0.5) {
      PRINT_ERROR("Invalid topology elastic properties for element %d.\n", assignment.first);
      return false;
    }

    distinct_factors[assignment.second].insert(factor->second);
  }

  std::map<int, int> budgets;
  std::size_t reserved = 0;

  for (const auto& entry : distinct_factors) {
    int budget = static_cast<int>(entry.second.size());
    if (quantize) {
      if (entry.second.size() == 1) {
        budget = 1;
      } else {
        budget = 2;
      }
    }
    budgets.emplace(entry.first, budget);
    reserved += static_cast<std::size_t>(budget);
  }

  if (reserved > static_cast<std::size_t>(material_cap)) {
    PRINT_ERROR("Topology material cap %d cannot preserve the base materials and stiffness endpoints, or saved grouping exceeds its cap.\n", material_cap);
    return false;
  }

  int remaining = material_cap - static_cast<int>(reserved);

  while (quantize && remaining > 0) {
    bool allocated = false;

    for (auto& budget : budgets) {
      const auto& values = distinct_factors.find(budget.first)->second;

      if (static_cast<std::size_t>(budget.second) < values.size()) {
        ++budget.second;
        --remaining;
        allocated = true;

        if (remaining == 0) {
          break;
        }
      }
    }

    if (!allocated) {
      break;
    }
  }

  TopologyMaterialGrouping candidate;
  candidate.material_cap = material_cap;
  std::map<int, double> actual_factors;
  std::map<std::pair<int, double>, int> indices;

  for (const auto& assignment : data.element_base_material) {
    const double requested = requested_factors.find(assignment.first)->second;
    const auto& values = distinct_factors.find(assignment.second)->second;
    const int budget = budgets.find(assignment.second)->second;
    double applied = requested;

    if (quantize && values.size() > static_cast<std::size_t>(budget)) {
      const double minimum = *values.begin();
      const double maximum = *values.rbegin();
      const double log_minimum = std::log(minimum);
      const double log_range = std::log(maximum) - log_minimum;

      if (!std::isfinite(log_range) || log_range <= 0.0) {
        PRINT_ERROR("Invalid topology stiffness range for base material %d.\n", assignment.second);
        return false;
      }

      const double position = std::clamp((std::log(requested) - log_minimum) / log_range, 0.0, 1.0);
      const int bin = std::clamp(static_cast<int>(std::llround(position * (budget - 1))), 0, budget - 1);

      if (bin == 0) {
        applied = minimum;
      }
      else if (bin == budget - 1) {
        applied = maximum;
      }
      else {
        applied = std::clamp(std::exp(log_minimum + log_range * static_cast<double>(bin) / static_cast<double>(budget - 1)), minimum, maximum);
      }
    }

    const double modulus = data.base_materials.find(assignment.second)->second.youngs_modulus * applied;

    if (!std::isfinite(applied) || applied <= 0.0 || applied > 1.0 || !std::isfinite(modulus) || modulus <= 0.0) {
      PRINT_ERROR("Invalid grouped topology modulus for element %d.\n", assignment.first);
      return false;
    }

    const std::pair<int, double> key{assignment.second, applied};
    auto group = indices.find(key);

    if (group == indices.end()) {
      const int index = static_cast<int>(candidate.groups.size());
      candidate.groups.push_back({index + 1, assignment.second, applied});
      group = indices.emplace(key, index).first;
    }

    candidate.element_group.emplace(assignment.first, group->second);
    actual_factors.emplace(assignment.first, applied);
    candidate.maximum_relative_factor_error = (std::max)(candidate.maximum_relative_factor_error, std::abs(applied / requested - 1.0));
  }

  if (candidate.groups.size() > static_cast<std::size_t>(material_cap)) {
    PRINT_ERROR("Topology grouping exceeded material cap %d.\n", material_cap);
    return false;
  }

  grouping = std::move(candidate);
  applied_factors = std::move(actual_factors);
  return true;
}

bool CoreMaterials::ensure_topology_material_groups(int topology_id, int iteration_id) {
  const auto collection = topology_material_data.find(topology_id);

  if (collection == topology_material_data.end()) {
    PRINT_ERROR("Topology material collection %d is missing.\n", topology_id);
    return false;
  }

  auto& data = collection->second;
  const auto cap = data.iteration_material_caps.find(iteration_id);
  const auto factors = data.iteration_factors.find(iteration_id);

  if (cap == data.iteration_material_caps.end() || factors == data.iteration_factors.end()) {
    PRINT_ERROR("Grouped material data for topology %d iteration %d is missing.\n", topology_id, iteration_id);
    return false;
  }

  if (data.iteration_material_groups.count(iteration_id) != 0) {
    return true;
  }

  TopologyMaterialGrouping grouping;
  std::map<int, double> unchanged_factors;

  // Reconstruct historical applied factors exactly; never quantize on reload.
  if (!build_topology_material_groups(data, factors->second, cap->second, grouping, unchanged_factors, false)) {
    return false;
  }

  // Retain only one element-to-group cache per topology; old iterations rebuild exactly.
  data.iteration_material_groups.clear();
  data.iteration_material_groups.emplace(iteration_id, std::move(grouping));
  return true;
}

std::string CoreMaterials::get_topology_material_group_name(int topology_id, int group_id) const {
  std::ostringstream name;
  name << "CCX_TOPO_" << topology_id << "_G" << std::setfill('0') << std::setw(10) << group_id;
  return name.str();
}

bool CoreMaterials::get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name) {
  material_name.clear();
  const auto collection = topology_material_data.find(topology_id);

  if (collection == topology_material_data.end() || collection->second.element_base_material.count(element_id) == 0) {
    PRINT_ERROR("Topology material assignment for element %d is missing.\n", element_id);
    return false;
  }

  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (factors == collection->second.iteration_factors.end() || factors->second.count(element_id) == 0) {
    PRINT_ERROR("Topology iteration factor for element %d is missing.\n", element_id);
    return false;
  }

  // Projects saved before grouping retain their per-element material names.
  if (collection->second.iteration_material_caps.count(iteration_id) == 0) {
    material_name = get_topology_material_name(topology_id, element_id);
    return true;
  }

  if (!ensure_topology_material_groups(topology_id, iteration_id)) {
    return false;
  }

  const auto& grouping = collection->second.iteration_material_groups.find(iteration_id)->second;
  const auto group = grouping.element_group.find(element_id);

  if (group == grouping.element_group.end() || group->second < 0 || static_cast<std::size_t>(group->second) >= grouping.groups.size()) {
    PRINT_ERROR("Invalid topology material group for element %d.\n", element_id);
    return false;
  }

  material_name = get_topology_material_group_name(topology_id, grouping.groups[group->second].id);
  return true;
}

std::string CoreMaterials::get_topology_material_export(int topology_id,int iteration_id) {
  const auto collection = topology_material_data.find(topology_id);

  if (collection == topology_material_data.end()) {
    PRINT_ERROR("Topology material collection %d is missing.\n", topology_id);
    return "";
  }

  if (collection->second.iteration_material_caps.count(iteration_id) == 0) {
    return get_topology_material_export_legacy(topology_id, iteration_id);
  }

  if (!ensure_topology_material_groups(topology_id, iteration_id)) {
    return "";
  }

  const auto& grouping = collection->second.iteration_material_groups.find(iteration_id)->second;
  std::ostringstream output;
  output << "** TOPOLOGY MATERIAL GROUPS: RUN " << topology_id << ", ITERATION " << iteration_id << "\n";

  for (const auto& group : grouping.groups) {
    const auto material = collection->second.base_materials.find(group.base_material_id);

    if (material == collection->second.base_materials.end()) {
      PRINT_ERROR("Missing topology base material %d.\n", group.base_material_id);
      return "";
    }

    double modulus = 0.0;
    std::string conversionError;
    if (!framework_units().to_solver(material->second.youngs_modulus * group.stiffness_factor, SimUnits::Stress, "calculix", modulus, conversionError)) {
      PRINT_ERROR("%s\n", conversionError.c_str());
      return {};
    }

    if (!std::isfinite(modulus) || modulus <= 0.0) {
      PRINT_ERROR("Invalid modulus for topology material group %d.\n", group.id);
      return "";
    }

    const std::string modulus_text = ScientificNumber::format(modulus, 6);
    const std::string poisson_text = ScientificNumber::format(material->second.poissons_ratio, 6);

    if (modulus_text.size() > 20 || poisson_text.size() > 20) {
      PRINT_ERROR("Elastic values for topology material group %d exceed the CalculiX field width.\n", group.id);
      return "";
    }

    output << "** CUBIT_SIM_MATERIAL_ID=" << group.base_material_id << ";" << get_topology_material_group_name(topology_id, group.id) << "\n";
    output << "*MATERIAL, NAME=" << get_topology_material_group_name(topology_id, group.id) << "\n"
           << "*ELASTIC\n"
           << modulus_text << ", " << poisson_text << "\n";
  }

  return output.str();
}

std::string CoreMaterials::get_material_export_data(int topology_id, int iteration_id) {
  if (topology_id == -1 && iteration_id == -1) {
    return get_material_export();
  }

  if (topology_id <= 0 || iteration_id < 1) {
    PRINT_ERROR("Invalid topology export selection.\n");
    return {};
  }

  const std::string topology_data = get_topology_material_export(topology_id, iteration_id);

  if (topology_data.empty()) {
    return {};
  }

  const std::string baseMaterials = get_material_export();
  if (baseMaterials.empty()) {
    return {};
  }
  return baseMaterials + "\n" + topology_data;
}

bool CoreMaterials::check_topology_material_assignment(int topology_id, int iteration_id, int element_id) const {
  const auto collection_it = topology_material_data.find(topology_id);

  if (collection_it == topology_material_data.end()) {
    return false;
  }

  const auto& collection = collection_it->second;
  const auto assignment = collection.element_base_material.find(element_id);
  const auto iteration = collection.iteration_factors.find(iteration_id);

  if (assignment == collection.element_base_material.end() || iteration == collection.iteration_factors.end()) {
    return false;
  }

  return collection.base_materials.count(assignment->second) != 0 && iteration->second.count(element_id) != 0;
}

std::string CoreMaterials::get_material_name(int material_id) {
  const FrameworkMaterialsInterface::Material material = mat_iface->get_material(material_id);
  return mat_iface->get_material_name(material);
}


bool CoreMaterials::get_properties(int id, MaterialProperties& properties, std::string& error) {
  FrameworkMaterialsInterface presentation("calculix", MaterialValueSpace::Display);
  properties = {};
  error.clear();
  if (mat_iface == nullptr || !check_material_exists(id)) {
    error = "The selected material is unavailable.";
    return false;
  }
  properties.id = id;
  properties.name = get_material_name(id);
  const auto material = presentation.get_material(properties.name);
  properties.group = presentation.get_group_name(presentation.get_material_property_group(material));
  for (const auto& entry : presentation.get_group_properties()) {
    if (entry.size() < 2) {
      continue;
    }
    const auto property = presentation.get_property(entry[0]);
    MaterialPropertyValues values;
    values.name = entry[0];
    if (entry[1] == "1") {
      double scalar = 0;
      if (presentation.get_material_property_value(material, property, scalar)) {
        values.values.push_back({scalar});
      }
    }
    else if (entry[1] == "2") {
      MaterialVector vector;
      if (presentation.get_material_property_value(material, property, vector)) {
        values.values.push_back(vector);
      }
    }
    else if (entry[1] == "4") {
      MaterialMatrix matrix;
      if (presentation.get_material_property_value(material, property, matrix)) {
        values.values = matrix;
      }
    }
    if (!values.values.empty()) {
      properties.properties.push_back(values);
    }
  }
  return true;
}
