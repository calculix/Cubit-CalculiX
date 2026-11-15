#include "MaterialManagementItem.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp"
#include "CubitMessage.hpp"
#include <QApplication>
#include <QStyle>
#include <utility>
MaterialManagementItem::MaterialManagementItem(QTreeWidget* parent, std::string id) : QObject(), QTreeWidgetItem(parent), workspace(std::move(id)) {
  projectIdentity = framework_materials().project_id();
}
MaterialManagementItem::MaterialManagementItem(QTreeWidgetItem* parent, std::string id) : QObject(), QTreeWidgetItem(parent), workspace(std::move(id)) {
  projectIdentity = framework_materials().project_id();
}
MaterialManagementItem::~MaterialManagementItem() = default;
void MaterialManagementItem::initialize_project(QString id, QString name) {
  isProjectMaterial = true;
  material_id = id.toInt();
  material_id_qstring = id;
  material_name_qstring = name;
  material_name = name.toStdString();
  setText(0, name);
  setText(1, id);
  setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));
  refresh();
}
void MaterialManagementItem::initialize_library(const std::vector<std::string>& data) {
  if (data.size() < 4) {
    return;
  }
  library_name = data[0];
  library_name_qstring = QString::fromStdString(data[0]);
  library_description = data[1];
  library_description_qstring = QString::fromStdString(data[1]);
  hdf5path = data[2];
  isLibraryGroup = data[3] == "Group";
  isLibraryMaterial = data[3] == "Material";
  library_group = hdf5path;
  if (isLibraryMaterial) {
    library_group = hdf5path.substr(0, hdf5path.find_last_of('/'));
  }
  while (!library_group.empty() && library_group.front() == '/') {
    library_group.erase(library_group.begin());
  }
  library_group_qstring = QString::fromStdString(library_group);
  setText(0, library_name_qstring);
  setText(1, library_description_qstring);
  if (isLibraryGroup) {
    setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
  } else {
    setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));
  }
  refresh();
}
bool MaterialManagementItem::refresh() {
  MaterialData data;
  if (isProjectMaterial) {
    FrameworkMaterialsInterface materials(workspace);
    const auto* instance = materials.find(material_id);
    if (instance == nullptr) {
      validationError = "Material no longer exists in this workspace.";
      return false;
    }
    data = instance->data;
    material_name = data.name;
    material_name_qstring = QString::fromStdString(data.name);
    setText(0, material_name_qstring);
  } else if (isLibraryMaterial) {
    if (!framework_materials().library().read_material(workspace, library_name, library_group, data, validationError)) {
      PRINT_ERROR("%s\n", validationError.c_str());
      return false;
    }
  } else {
    isInitialized = true;
    return true;
  }
  original = data;
  draft = std::move(data);
  projectIdentity = framework_materials().project_id();
  validationError.clear();
  propertyErrors.clear();
  isInitialized = true;
  return true;
}
bool MaterialManagementItem::equal(const MaterialData& first, const MaterialData& second) {
  if (first.schema != second.schema || first.name != second.name || first.description != second.description || first.cards.size() != second.cards.size()) {
    return false;
  }
  for (const auto& entry : first.cards) {
    auto found = second.cards.find(entry.first);
    if (found == second.cards.end()) {
      return false;
    }
    const MaterialCard& left = entry.second;
    const MaterialCard& right = found->second;
    if (left.state != right.state || left.model != right.model || left.values.scalar != right.values.scalar || left.values.referenceTemperature != right.values.referenceTemperature || left.values.rows.size() != right.values.rows.size()) {
      return false;
    }
    for (std::size_t index = 0; index < left.values.rows.size(); ++index) {
      if (left.values.rows[index].values != right.values.rows[index].values) {
        return false;
      }
    }
  }
  return true;
}
bool MaterialManagementItem::hasChanges() const {
  return !validationError.empty() || !propertyErrors.empty() || !equal(original, draft);
}
std::string MaterialManagementItem::validationMessage() const {
  if (!validationError.empty()) {
    return validationError;
  }
  if (!propertyErrors.empty()) {
    return propertyErrors.begin()->second;
  }
  return {};
}
bool MaterialManagementItem::hasPropertyError(const std::string& property) const {
  return propertyErrors.count(property) != 0;
}
void MaterialManagementItem::setPropertyError(const std::string& property, const std::string& error) {
  propertyErrors[property] = error;
}
void MaterialManagementItem::clearPropertyError(const std::string& property) {
  propertyErrors.erase(property);
}
void MaterialManagementItem::setScalarPropertyGUI(const std::string& property, double value) {
  FrameworkMaterialsInterface materials(workspace);
  std::string error;
  if (!materials.write_scalar(draft, property, value, error)) {
    setPropertyError(property, error);
    PRINT_ERROR("%s\n", error.c_str());
  } else {
    clearPropertyError(property);
  }
}
double MaterialManagementItem::getScalarPropertyGUI(const std::string& property) const {
  FrameworkMaterialsInterface materials(workspace);
  double value = 0.0;
  std::string error;
  if (!materials.read_scalar(draft, property, value, error)) {
    PRINT_ERROR("%s\n", error.c_str());
  }
  return value;
}
void MaterialManagementItem::setMatrixPropertyGUI(const std::string& property, const std::vector<std::vector<double>>& values) {
  FrameworkMaterialsInterface materials(workspace);
  std::string error;
  if (!materials.write_table(draft, property, values, error)) {
    setPropertyError(property, error);
    PRINT_ERROR("%s\n", error.c_str());
  } else {
    clearPropertyError(property);
  }
}
std::vector<std::vector<double>> MaterialManagementItem::getMatrixPropertyGUI(const std::string& property) const {
  FrameworkMaterialsInterface materials(workspace);
  std::vector<std::vector<double>> values;
  std::string error;
  if (!materials.read_table(draft, property, values, error)) {
    PRINT_ERROR("%s\n", error.c_str());
  }
  return values;
}
bool MaterialManagementItem::modelSelectedGUI(const std::string& card, const std::string& model) const {
  auto found = draft.cards.find(card);
  return found != draft.cards.end() && found->second.model == model;
}
void MaterialManagementItem::selectModelGUI(const std::string& card, const std::string& model) {
  const auto* definition = framework_materials().definitions().card(card);
  if (definition == nullptr || framework_materials().definitions().model(*definition, model) == nullptr) {
    setPropertyError(card, "Unknown material model.");
    PRINT_ERROR("Unknown material model.\n");
    return;
  }
  MaterialCard& value = draft.cards[definition->key];
  if (value.model != model) {
    value.model = model;
    value.values = {};
    for (auto error = propertyErrors.begin(); error != propertyErrors.end();) {
      if (error->first.compare(0, definition->prefix.size(), definition->prefix) == 0) {
        error = propertyErrors.erase(error);
      } else {
        ++error;
      }
    }
  }
  clearPropertyError(card);
}
bool MaterialManagementItem::cardActiveGUI(const std::string& card) const {
  auto found = draft.cards.find(card);
  return found != draft.cards.end() && found->second.state == MaterialCardState::Active;
}
void MaterialManagementItem::setCardActiveGUI(const std::string& card, bool active) {
  const auto* definition = framework_materials().definitions().card(card);
  if (definition == nullptr || definition->models.empty()) {
    return;
  }
  MaterialCard& value = draft.cards[definition->key];
  if (value.model.empty()) {
    value.model = definition->models.front().key;
  }
  if (active) {
    value.state = MaterialCardState::Active;
  } else {
    value.state = MaterialCardState::Inactive;
  }
}
std::optional<double> MaterialManagementItem::referenceTemperatureGUI() const {
  auto found = draft.cards.find("expansion");
  if (found == draft.cards.end() || !found->second.values.referenceTemperature.has_value()) {
    return {};
  }
  return getScalarPropertyGUI("CCX_EXPANSION_ZERO");
}
void MaterialManagementItem::setReferenceTemperatureGUI(double value) {
  setScalarPropertyGUI("CCX_EXPANSION_ZERO", value);
}
void MaterialManagementItem::clearReferenceTemperatureGUI() {
  auto found = draft.cards.find("expansion");
  if (found != draft.cards.end()) {
    found->second.values.referenceTemperature.reset();
  }
  clearPropertyError("CCX_EXPANSION_ZERO");
}
