#include "CoreFrameworkMaterials.hpp"
#include "CoreFrameworkMaterialLibrary.hpp"
#include <QUuid>
CoreFrameworkMaterials::CoreFrameworkMaterials() : materialLibrary(std::make_unique<CoreFrameworkMaterialLibrary>(materialDefinitions)) {
  projectId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}
CoreFrameworkMaterials::~CoreFrameworkMaterials() = default;
const MaterialDefinitions& CoreFrameworkMaterials::definitions() const noexcept {
  return materialDefinitions;
}
WorkspaceMaterialStore& CoreFrameworkMaterials::workspace(const std::string& id) {
  auto found = workspaces.find(id);
  if (found == workspaces.end()) {
    found = workspaces.emplace(id, std::make_unique<WorkspaceMaterialStore>(id, materialDefinitions)).first;
  }
  return *found->second;
}
CoreFrameworkMaterialLibrary& CoreFrameworkMaterials::library() {
  return *materialLibrary;
}
void CoreFrameworkMaterials::reset(const std::string& id) {
  workspace(id).clear();
}
const std::string& CoreFrameworkMaterials::project_id() const noexcept {
  return projectId;
}
void CoreFrameworkMaterials::begin_project() {
  for (auto& entry : workspaces) {
    entry.second->reset();
  }
  projectId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}
bool CoreFrameworkMaterials::set_project_id(const std::string& id, std::string& error) {
  if (QUuid(QString::fromStdString(id)).isNull()) {
    error = "Invalid material project identity.";
    return false;
  }
  projectId = id;
  return true;
}
CoreFrameworkMaterials& framework_materials() {
  static CoreFrameworkMaterials materials;
  return materials;
}
