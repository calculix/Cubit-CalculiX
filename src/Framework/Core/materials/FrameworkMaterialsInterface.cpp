#include "FrameworkMaterialsInterface.hpp"
#include "CoreFrameworkMaterialLibrary.hpp"
#include "MaterialProjectPersistence.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <utility>
namespace {
bool report(bool success, const std::string& error) {
  if (!success) {
    PRINT_ERROR("%s\n", error.c_str());
  }
  return success;
}
MaterialCard make_card(const FrameworkMaterialCardDefinition& definition) {
  MaterialCard card;
  if (!definition.models.empty()) {
    card.model = definition.models.front().key;
  }
  return card;
}
}
FrameworkMaterialsInterface::FrameworkMaterialsInterface(std::string workspace, MaterialValueSpace values) : workspaceId(std::move(workspace)), space(values) {
}
FrameworkMaterialsInterface::~FrameworkMaterialsInterface() = default;
const MaterialInstance* FrameworkMaterialsInterface::find(int id) const {
  return framework_materials().workspace(workspaceId).find({workspaceId, id});
}
int FrameworkMaterialsInterface::get_material(int id) const {
  if (find(id) == nullptr) {
    return 0;
  }
  return id;
}
int FrameworkMaterialsInterface::get_material(const std::string& name) const {
  return framework_materials().workspace(workspaceId).find_name(name);
}
int FrameworkMaterialsInterface::get_material_id(int material) const {
  return get_material(material);
}
std::string FrameworkMaterialsInterface::get_material_name(int material) const {
  const auto* instance = find(material);
  if (instance == nullptr) {
    return {};
  }
  return instance->data.name;
}
std::string FrameworkMaterialsInterface::get_material_property_group(int material) const {
  if (find(material) == nullptr) {
    return {};
  }
  return "SIM_CCX";
}
std::string FrameworkMaterialsInterface::get_group_name(const std::string& group) const {
  return group;
}
std::string FrameworkMaterialsInterface::get_property(const std::string& property) const {
  return property;
}
std::vector<int> FrameworkMaterialsInterface::get_material_ids() const {
  std::vector<int> ids;
  for (const auto& entry : framework_materials().workspace(workspaceId).all()) {
    ids.push_back(entry.first);
  }
  return ids;
}
std::vector<std::vector<std::string>> FrameworkMaterialsInterface::get_group_properties() const {
  return framework_materials().definitions().legacy_properties();
}
int FrameworkMaterialsInterface::get_property_size(const std::string& property) const {
  for (const auto& entry : get_group_properties()) {
    if (entry.front() == property) {
      if (entry.size() > 2) {
        return std::stoi(entry[2]);
      }
      return 1;
    }
  }
  return -1;
}
bool FrameworkMaterialsInterface::convert(double input, const MaterialModelDefinition& model, std::size_t column, const std::vector<double>& row, bool toCanonical, double& output, std::string& error) const {
  if (!std::isfinite(input) || column >= model.columns.size()) {
    error = "Invalid material value or column.";
    return false;
  }
  if (space == MaterialValueSpace::Canonical) {
    output = input;
    return true;
  }
  const std::string& quantity = model.columns[column].quantity;
  if (quantity == "creep_coefficient") {
    if (row.size() < 3 || !std::isfinite(row[1]) || !std::isfinite(row[2])) {
      error = "Creep coefficient requires finite n and m exponents.";
      return false;
    }
    SimUnits::Dimension dimension = SimUnits::Dimensionless;
    for (std::size_t index = 0; index < dimension.size(); ++index) {
      dimension[index] = -row[1] * SimUnits::Stress[index] - (row[2] + 1.0) * SimUnits::Time[index];
    }
    if (space == MaterialValueSpace::Display) {
      std::string stressUnit;
      std::string timeUnit;
      double stressScale = 0.0;
      double timeScale = 0.0;
      if (!framework_units().display_unit(workspaceId, "stress", stressUnit, error) || !framework_units().display_unit(workspaceId, "time", timeUnit, error)) {
        return false;
      }
      if (!framework_units().to_si(1.0, stressUnit, SimUnits::Stress, SimUnits::UnitRole::Linear, stressScale, error) || !framework_units().to_si(1.0, timeUnit, SimUnits::Time, SimUnits::UnitRole::Linear, timeScale, error)) {
        return false;
      }
      const double scale = std::pow(stressScale, -row[1]) * std::pow(timeScale, -(row[2] + 1.0));
      if (!std::isfinite(scale) || scale <= 0.0) {
        error = "Creep coefficient unit scale cannot be represented.";
        return false;
      }
      if (toCanonical) {
        output = input * scale;
      } else {
        output = input / scale;
      }
      if (!std::isfinite(output)) {
        error = "Converted creep coefficient is not finite.";
        return false;
      }
      return true;
    }
    if (toCanonical) {
      return framework_units().from_solver(input, dimension, workspaceId, output, error);
    }
    return framework_units().to_solver(input, dimension, workspaceId, output, error);
  }
  SimUnits::QuantityUnit definition;
  if (!framework_units().quantity(quantity, definition, error)) {
    return false;
  }
  if (space == MaterialValueSpace::Solver && definition.role != SimUnits::UnitRole::Angle) {
    if (toCanonical) {
      return framework_units().from_solver(input, definition.dimension, workspaceId, output, error);
    }
    return framework_units().to_solver(input, definition.dimension, workspaceId, output, error);
  }
  std::string unit;
  if (space == MaterialValueSpace::Solver && definition.role == SimUnits::UnitRole::Angle) {
    unit = "deg";
  } else if (!framework_units().display_unit(workspaceId, quantity, unit, error)) {
    return false;
  }
  if (toCanonical) {
    return framework_units().to_si(input, unit, definition.dimension, definition.role, output, error);
  }
  return framework_units().from_si(input, unit, definition.dimension, definition.role, output, error);
}
bool FrameworkMaterialsInterface::read_scalar(const MaterialData& material, const std::string& property, double& value, std::string& error) const {
  value = 0.0;
  const auto& catalog = framework_materials().definitions();
  for (const auto& definition : catalog.cards()) {
    auto found = material.cards.find(definition.key);
    if (property == definition.prefix + "USE_CARD") {
      if (found != material.cards.end() && found->second.state == MaterialCardState::Active) {
        value = 1.0;
      }
      return true;
    }
  }
  auto expansion = material.cards.find("expansion");
  if (property == "CCX_EXPANSION_ZERO_USE_CARD") {
    if (expansion != material.cards.end() && expansion->second.values.referenceTemperature.has_value()) {
      value = 1.0;
    }
    return true;
  }
  if (property == "CCX_EXPANSION_ZERO") {
    if (expansion == material.cards.end() || !expansion->second.values.referenceTemperature.has_value()) {
      return true;
    }
    MaterialModelDefinition definition;
    definition.columns.push_back({"reference_temperature", "Reference temperature", "temperature"});
    return convert(*expansion->second.values.referenceTemperature, definition, 0, {}, false, value, error);
  }
  const FrameworkMaterialCardDefinition* definition = nullptr;
  const auto* model = catalog.property(property, definition);
  if (model == nullptr || definition == nullptr) {
    error = "Unknown scalar material property: " + property;
    return false;
  }
  auto found = material.cards.find(definition->key);
  if (property == model->selectionProperty) {
    if (found != material.cards.end() && found->second.model == model->key) {
      value = 1.0;
    }
    return true;
  }
  if (!model->scalar) {
    error = "Material property requires a table: " + property;
    return false;
  }
  if (found == material.cards.end() || found->second.model != model->key || !found->second.values.scalar.has_value()) {
    return true;
  }
  return convert(*found->second.values.scalar, *model, 0, {}, false, value, error);
}
bool FrameworkMaterialsInterface::read_table(const MaterialData& material, const std::string& property, std::vector<std::vector<double>>& values, std::string& error) const {
  values.clear();
  const FrameworkMaterialCardDefinition* definition = nullptr;
  const auto* model = framework_materials().definitions().property(property, definition);
  if (model == nullptr || definition == nullptr || model->scalar || model->property != property) {
    error = "Unknown table material property: " + property;
    return false;
  }
  auto found = material.cards.find(definition->key);
  if (found == material.cards.end() || found->second.model != model->key) {
    return true;
  }
  std::vector<std::vector<double>> candidate;
  for (const auto& source : found->second.values.rows) {
    std::vector<double> row;
    for (std::size_t column = 0; column < source.values.size(); ++column) {
      double converted = 0.0;
      if (!convert(source.values[column], *model, column, source.values, false, converted, error)) {
        return false;
      }
      row.push_back(converted);
    }
    candidate.push_back(std::move(row));
  }
  values = std::move(candidate);
  return true;
}
bool FrameworkMaterialsInterface::get_material_property_value(int material, const std::string& property, double& value) const {
  const auto* instance = find(material);
  if (instance == nullptr) {
    value = 0.0;
    return false;
  }
  std::string error;
  return report(read_scalar(instance->data, property, value, error), error);
}
bool FrameworkMaterialsInterface::get_material_property_value(int material, const std::string& property, std::vector<std::vector<double>>& values) const {
  const auto* instance = find(material);
  if (instance == nullptr) {
    values.clear();
    return false;
  }
  std::string error;
  return report(read_table(instance->data, property, values, error), error);
}
bool FrameworkMaterialsInterface::get_material_property_value(int material, const std::string& property, std::vector<double>& values) const {
  double value = 0.0;
  values.clear();
  if (!get_material_property_value(material, property, value)) {
    return false;
  }
  values.push_back(value);
  return true;
}
bool FrameworkMaterialsInterface::write_scalar(MaterialData& material, const std::string& property, double value, std::string& error) const {
  if (!std::isfinite(value)) {
    error = "Material scalar must be finite.";
    return false;
  }
  const auto& catalog = framework_materials().definitions();
  for (const auto& definition : catalog.cards()) {
    if (property == definition.prefix + "USE_CARD") {
      if (value != 0.0 && value != 1.0) {
        error = "Material activation must be active or inactive.";
        return false;
      }
      auto found = material.cards.find(definition.key);
      if (found == material.cards.end()) {
        found = material.cards.emplace(definition.key, make_card(definition)).first;
      }
      if (value == 1.0) {
        found->second.state = MaterialCardState::Active;
      } else {
        found->second.state = MaterialCardState::Inactive;
      }
      return true;
    }
  }
  if (property == "CCX_EXPANSION_ZERO" || property == "CCX_EXPANSION_ZERO_USE_CARD") {
    const auto* definition = catalog.card("expansion");
    auto found = material.cards.find("expansion");
    if (found == material.cards.end()) {
      found = material.cards.emplace("expansion", make_card(*definition)).first;
    }
    if (property == "CCX_EXPANSION_ZERO_USE_CARD") {
      if (value == 0.0) {
        found->second.values.referenceTemperature.reset();
      } else if (value == 1.0) {
        if (!found->second.values.referenceTemperature.has_value()) {
          found->second.values.referenceTemperature = 0.0;
        }
      } else {
        error = "Reference temperature presence must be zero or one.";
        return false;
      }
      return true;
    }
    MaterialModelDefinition scalar;
    scalar.columns.push_back({"reference_temperature", "Reference temperature", "temperature"});
    double converted = 0.0;
    if (!convert(value, scalar, 0, {}, true, converted, error)) {
      return false;
    }
    found->second.values.referenceTemperature = converted;
    return true;
  }
  const FrameworkMaterialCardDefinition* definition = nullptr;
  const auto* model = catalog.property(property, definition);
  if (model == nullptr || definition == nullptr) {
    error = "Unknown scalar material property: " + property;
    return false;
  }
  if (property == model->selectionProperty && value == 0.0) {
    return true;
  }
  if (property == model->selectionProperty && value != 1.0) {
    error = "Material model selection must be zero or one.";
    return false;
  }
  if (property != model->selectionProperty && !model->scalar) {
    error = "Material property requires a table: " + property;
    return false;
  }
  auto found = material.cards.find(definition->key);
  if (found == material.cards.end()) {
    found = material.cards.emplace(definition->key, make_card(*definition)).first;
  }
  if (found->second.model != model->key) {
    found->second.model = model->key;
    found->second.values = {};
  }
  if (property == model->selectionProperty) {
    return true;
  }
  double converted = 0.0;
  if (!convert(value, *model, 0, {}, true, converted, error)) {
    return false;
  }
  found->second.values.scalar = converted;
  return true;
}
bool FrameworkMaterialsInterface::write_table(MaterialData& material, const std::string& property, const std::vector<std::vector<double>>& values, std::string& error) const {
  const FrameworkMaterialCardDefinition* definition = nullptr;
  const auto* model = framework_materials().definitions().property(property, definition);
  if (model == nullptr || definition == nullptr || model->scalar || model->property != property) {
    error = "Unknown table material property: " + property;
    return false;
  }
  std::vector<MaterialTableRow> convertedRows;
  for (const auto& row : values) {
    bool validWidth = row.size() == model->columns.size();
    if (!model->columns.empty() && model->columns.back().optional && row.size() + 1 == model->columns.size()) {
      validWidth = true;
    }
    if (!validWidth) {
      error = "Material table has an incorrect column count: " + property;
      return false;
    }
    MaterialTableRow converted;
    for (std::size_t column = 0; column < row.size(); ++column) {
      double value = 0.0;
      if (!convert(row[column], *model, column, row, true, value, error)) {
        return false;
      }
      converted.values.push_back(value);
    }
    convertedRows.push_back(std::move(converted));
  }
  auto found = material.cards.find(definition->key);
  if (found == material.cards.end()) {
    found = material.cards.emplace(definition->key, make_card(*definition)).first;
  }
  if (found->second.model != model->key) {
    found->second.model = model->key;
    found->second.values = {};
  }
  found->second.values.rows = std::move(convertedRows);
  return true;
}
bool FrameworkMaterialsInterface::set_scalar(int material, const std::string& property, double value, std::string& error) {
  const auto* instance = find(material);
  if (instance == nullptr) {
    error = "Material does not exist in this workspace.";
    return false;
  }
  MaterialData data = instance->data;
  if (!write_scalar(data, property, value, error)) {
    return false;
  }
  return framework_materials().workspace(workspaceId).replace({workspaceId, material}, std::move(data), error);
}
bool FrameworkMaterialsInterface::set_table(int material, const std::string& property, const std::vector<std::vector<double>>& values, std::string& error) {
  const auto* instance = find(material);
  if (instance == nullptr) {
    error = "Material does not exist in this workspace.";
    return false;
  }
  MaterialData data = instance->data;
  if (!write_table(data, property, values, error)) {
    return false;
  }
  return framework_materials().workspace(workspaceId).replace({workspaceId, material}, std::move(data), error);
}
bool FrameworkMaterialsInterface::create_material(const std::string& name, int& id, std::string& error, int requestedId) {
  MaterialData data;
  data.schema = workspaceId;
  data.name = name;
  for (const auto& definition : framework_materials().definitions().cards()) {
    data.cards.emplace(definition.key, make_card(definition));
  }
  MaterialRef reference;
  if (!framework_materials().workspace(workspaceId).create(std::move(data), requestedId, reference, error)) {
    return false;
  }
  id = reference.id;
  return true;
}
bool FrameworkMaterialsInterface::delete_material(int id, std::string& error) {
  return framework_materials().workspace(workspaceId).erase({workspaceId, id}, error);
}
bool FrameworkMaterialsInterface::rename_material(int id, const std::string& name, std::string& error) {
  return framework_materials().workspace(workspaceId).rename({workspaceId, id}, name, error);
}
bool FrameworkMaterialsInterface::set_card_state(int id, const std::string& card, MaterialCardState state, std::string& error) {
  const auto* definition = framework_materials().definitions().card(card);
  if (definition == nullptr) {
    error = "Unknown material card: " + card;
    return false;
  }
  double value = 0.0;
  if (state == MaterialCardState::Active) {
    const auto* instance = find(id);
    if (instance == nullptr) {
      error = "Material does not exist in this workspace.";
      return false;
    }
    MaterialData validation;
    validation.schema = instance->data.schema;
    validation.name = instance->data.name;
    auto found = instance->data.cards.find(definition->key);
    if (found == instance->data.cards.end()) {
      error = "Configure the material card before activating it.";
      return false;
    }
    validation.cards.emplace(found->first, found->second);
    validation.cards.at(found->first).state = MaterialCardState::Active;
    if (!framework_materials().definitions().validate(validation, true, error)) {
      return false;
    }
    value = 1.0;
  }
  return set_scalar(id, definition->prefix + "USE_CARD", value, error);
}
bool FrameworkMaterialsInterface::set_model(int id, const std::string& card, const std::string& modelName, std::string& error) {
  const auto* instance = find(id);
  const auto* definition = framework_materials().definitions().card(card);
  if (instance == nullptr || definition == nullptr) {
    error = "Unknown material or card.";
    return false;
  }
  const auto* model = framework_materials().definitions().model(*definition, modelName);
  if (model == nullptr) {
    error = "Unknown material model: " + modelName;
    return false;
  }
  MaterialData data = instance->data;
  MaterialCard& selected = data.cards[definition->key];
  if (selected.model != model->key) {
    selected.model = model->key;
    selected.values = {};
  }
  return framework_materials().workspace(workspaceId).replace({workspaceId, id}, std::move(data), error);
}
bool FrameworkMaterialsInterface::validate_materials(std::string& error) const {
  for (const auto& entry : framework_materials().workspace(workspaceId).all()) {
    if (!framework_materials().definitions().validate(entry.second.data, true, error)) {
      error = entry.second.data.name + ": " + error;
      return false;
    }
  }
  return true;
}
bool FrameworkMaterialsInterface::save(HDF5Tool& file, const std::string& group, std::string& error) const {
  return MaterialProjectPersistence::save(file, group, framework_materials().workspace(workspaceId), error);
}
bool FrameworkMaterialsInterface::load(HDF5Tool& file, const std::string& group, std::string& error) {
  return MaterialProjectPersistence::load(file, group, framework_materials().workspace(workspaceId), framework_materials().definitions(), error);
}
void FrameworkMaterialsInterface::reset() {
  framework_materials().reset(workspaceId);
}
const std::string& FrameworkMaterialsInterface::workspace() const noexcept {
  return workspaceId;
}
MaterialValueSpace FrameworkMaterialsInterface::value_space() const noexcept {
  return space;
}
std::vector<std::vector<std::string>> FrameworkMaterialsInterface::get_material_tree_data() const {
  std::vector<std::vector<std::string>> rows;
  for (const auto& entry : framework_materials().workspace(workspaceId).all()) {
    rows.push_back({std::to_string(entry.first), entry.second.data.name});
  }
  return rows;
}
std::vector<std::vector<std::string>> FrameworkMaterialsInterface::get_material_group_properties() const {
  return get_group_properties();
}
