#pragma once
#include "MaterialsExport.hpp"
#include "MaterialInstance.hpp"
#include <map>
#include <string>
#include <vector>
class MaterialDefinitions;
class CUBITSIM_MATERIALS_EXPORT WorkspaceMaterialStore {
public:
  WorkspaceMaterialStore(std::string workspace, const MaterialDefinitions& definitions);
  ~WorkspaceMaterialStore();
  const std::string& workspace() const noexcept;
  const std::map<int, MaterialInstance>& all() const noexcept;
  const MaterialInstance* find(const MaterialRef& reference) const noexcept;
  MaterialInstance* find(const MaterialRef& reference) noexcept;
  int find_name(const std::string& name) const noexcept;
  bool create(MaterialData data, int requestedId, MaterialRef& reference, std::string& error);
  bool replace(const MaterialRef& reference, MaterialData data, std::string& error);
  bool rename(const MaterialRef& reference, const std::string& name, std::string& error);
  bool erase(const MaterialRef& reference, std::string& error);
  bool restore(std::map<int, MaterialInstance> data, int nextId, std::string& error);
  int next_id() const noexcept;
  void clear() noexcept;
  void reset() noexcept;
  void swap_state(WorkspaceMaterialStore& other) noexcept;
private:
  std::string workspaceId;
  const MaterialDefinitions& definitions;
  int nextId = 1;
  std::map<int, MaterialInstance> materials;
};
