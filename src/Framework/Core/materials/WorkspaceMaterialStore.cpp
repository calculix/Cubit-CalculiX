#include "WorkspaceMaterialStore.hpp"
#include "MaterialDefinitions.hpp"
#include <cassert>
#include <limits>
#include <set>
#include <utility>
WorkspaceMaterialStore::WorkspaceMaterialStore(std::string workspace, const MaterialDefinitions& catalog) : workspaceId(std::move(workspace)), definitions(catalog) {
}
WorkspaceMaterialStore::~WorkspaceMaterialStore() = default;
const std::string& WorkspaceMaterialStore::workspace() const noexcept {
  return workspaceId;
}
const std::map<int, MaterialInstance>& WorkspaceMaterialStore::all() const noexcept {
  return materials;
}
const MaterialInstance* WorkspaceMaterialStore::find(const MaterialRef& reference) const noexcept {
  if (reference.workspace != workspaceId) {
    return nullptr;
  }
  auto found = materials.find(reference.id);
  if (found == materials.end()) {
    return nullptr;
  }
  return &found->second;
}
MaterialInstance* WorkspaceMaterialStore::find(const MaterialRef& reference) noexcept {
  if (reference.workspace != workspaceId) {
    return nullptr;
  }
  auto found = materials.find(reference.id);
  if (found == materials.end()) {
    return nullptr;
  }
  return &found->second;
}
int WorkspaceMaterialStore::find_name(const std::string& name) const noexcept {
  for (const auto& entry : materials) {
    if (entry.second.data.name == name) {
      return entry.first;
    }
  }
  return 0;
}
bool WorkspaceMaterialStore::create(MaterialData data, int requestedId, MaterialRef& reference, std::string& error) {
  if (workspaceId != data.schema || !definitions.validate(data, false, error)) {
    if (error.empty()) {
      error = "Material schema does not belong to this workspace.";
    }
    return false;
  }
  if (find_name(data.name) != 0) {
    error = "Material name already exists in this workspace: " + data.name;
    return false;
  }
  int id = requestedId;
  if (id == 0) {
    id = nextId;
  }
  if (id < nextId || id >= std::numeric_limits<int>::max() || materials.count(id) != 0) {
    error = "Material ID is invalid or already exists in this workspace.";
    return false;
  }
  MaterialInstance instance;
  instance.reference = {workspaceId, id};
  instance.data = std::move(data);
  materials.emplace(id, std::move(instance));
  if (id >= nextId) {
    nextId = id + 1;
  }
  reference = {workspaceId, id};
  return true;
}
bool WorkspaceMaterialStore::replace(const MaterialRef& reference, MaterialData data, std::string& error) {
  MaterialInstance* instance = find(reference);
  if (instance == nullptr) {
    error = "Material does not exist in this workspace.";
    return false;
  }
  if (data.schema != workspaceId || !definitions.validate(data, false, error)) {
    if (error.empty()) {
      error = "Material schema does not belong to this workspace.";
    }
    return false;
  }
  const int duplicate = find_name(data.name);
  if (duplicate != 0 && duplicate != reference.id) {
    error = "Material name already exists in this workspace: " + data.name;
    return false;
  }
  instance->data = std::move(data);
  return true;
}
bool WorkspaceMaterialStore::rename(const MaterialRef& reference, const std::string& name, std::string& error) {
  const MaterialInstance* instance = find(reference);
  if (instance == nullptr) {
    error = "Material does not exist in this workspace.";
    return false;
  }
  MaterialData data = instance->data;
  data.name = name;
  return replace(reference, std::move(data), error);
}
bool WorkspaceMaterialStore::erase(const MaterialRef& reference, std::string& error) {
  if (find(reference) == nullptr) {
    error = "Material does not exist in this workspace.";
    return false;
  }
  materials.erase(reference.id);
  return true;
}
bool WorkspaceMaterialStore::restore(std::map<int, MaterialInstance> data, int restoredNextId, std::string& error) {
  if (restoredNextId < 1) {
    error = "Invalid next material ID.";
    return false;
  }
  std::set<std::string> names;
  for (const auto& entry : data) {
    const MaterialInstance& material = entry.second;
    if (entry.first < 1 || entry.first >= restoredNextId || material.reference.id != entry.first || material.reference.workspace != workspaceId || material.data.schema != workspaceId || !names.insert(material.data.name).second) {
      error = "Invalid workspace material identities.";
      return false;
    }
    if (!definitions.validate(material.data, false, error)) {
      return false;
    }
  }
  materials = std::move(data);
  nextId = restoredNextId;
  return true;
}
int WorkspaceMaterialStore::next_id() const noexcept {
  return nextId;
}
void WorkspaceMaterialStore::clear() noexcept {
  materials.clear();
}
void WorkspaceMaterialStore::reset() noexcept {
  materials.clear();
  nextId = 1;
}
void WorkspaceMaterialStore::swap_state(WorkspaceMaterialStore& other) noexcept {
  assert(workspaceId == other.workspaceId);
  assert(&definitions == &other.definitions);
  materials.swap(other.materials);
  std::swap(nextId, other.nextId);
}
