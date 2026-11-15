#pragma once
#include "MaterialsExport.hpp"
#include "MaterialDefinitions.hpp"
#include "WorkspaceMaterialStore.hpp"
#include <map>
#include <memory>
#include <string>
class CoreFrameworkMaterialLibrary;
class CUBITSIM_MATERIALS_EXPORT CoreFrameworkMaterials {
public:
  CoreFrameworkMaterials();
  ~CoreFrameworkMaterials();
  const MaterialDefinitions& definitions() const noexcept;
  WorkspaceMaterialStore& workspace(const std::string& workspace);
  CoreFrameworkMaterialLibrary& library();
  void reset(const std::string& workspace);
  const std::string& project_id() const noexcept;
  void begin_project();
  bool set_project_id(const std::string& id, std::string& error);
private:
  MaterialDefinitions materialDefinitions;
  std::map<std::string, std::unique_ptr<WorkspaceMaterialStore>> workspaces;
  std::unique_ptr<CoreFrameworkMaterialLibrary> materialLibrary;
  std::string projectId;
};
CUBITSIM_MATERIALS_EXPORT CoreFrameworkMaterials& framework_materials();
