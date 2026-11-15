#pragma once
#include "MaterialsExport.hpp"
#include "MaterialData.hpp"
#include <string>
class HDF5Tool;
class WorkspaceMaterialStore;
class MaterialDefinitions;
class CUBITSIM_MATERIALS_EXPORT MaterialProjectPersistence {
public:
  MaterialProjectPersistence();
  ~MaterialProjectPersistence();
  static bool write_material(HDF5Tool& file, const std::string& group, const MaterialData& material, std::string& error);
  static bool read_material(HDF5Tool& file, const std::string& group, const MaterialDefinitions& definitions, MaterialData& material, std::string& error);
  static bool save(HDF5Tool& file, const std::string& group, const WorkspaceMaterialStore& store, std::string& error);
  static bool load(HDF5Tool& file, const std::string& group, WorkspaceMaterialStore& store, const MaterialDefinitions& definitions, std::string& error);
};
