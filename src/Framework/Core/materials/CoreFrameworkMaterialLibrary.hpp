#pragma once
#include "MaterialsExport.hpp"
#include "MaterialLibraryEntry.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>
class MaterialDefinitions;
class HDF5Tool;
class UserOptionsInterface;
class CUBITSIM_MATERIALS_EXPORT CoreFrameworkMaterialLibrary {
public:
  explicit CoreFrameworkMaterialLibrary(const MaterialDefinitions& definitions);
  ~CoreFrameworkMaterialLibrary();
  bool set_filename(const std::string& filename, std::string& error);
  const std::string& filename() const noexcept;
  bool initialize(std::string& error);
  bool initialize(UserOptionsInterface& options, std::string& error);
  bool set_file_option(const std::string& filename, std::string& error);
  void shutdown() noexcept;
  bool create_group(const std::string& name, std::string& error);
  bool modify_group(const std::string& name, const std::string& data, int mode, std::string& error);
  bool delete_group(const std::string& name, std::string& error);
  bool create_material(const std::string& workspace, const std::string& name, const std::string& group, std::string& error);
  bool replace_material(const std::string& workspace, const std::string& name, const std::string& group, MaterialData data, std::string& error);
  bool modify_material(const std::string& workspace, const std::string& name, const std::string& group, const std::string& data, int mode, const std::vector<double>& values, std::string& error);
  bool delete_material(const std::string& workspace, const std::string& name, const std::string& group, std::string& error);
  bool read_material(const std::string& workspace, const std::string& name, const std::string& group, MaterialData& material, std::string& error) const;
  bool export_material(const std::string& workspace, int id, const std::string& name, const std::string& group, std::string& error);
  bool import_material(const std::string& workspace, const std::string& name, const std::string& group, const std::string& localName, int& id, std::string& error);
  bool import_library(const std::string& sourceFilename, std::string& error);
  bool tree_data(const std::string& workspace, std::vector<std::vector<std::string>>& data, std::string& error) const;
private:
  using Groups = std::map<std::string, std::string>;
  using Entries = std::map<std::string, MaterialLibraryEntry>;
  using Mutation = std::function<bool(Groups&, Entries&, std::string&)>;
  bool mutate(const Mutation& mutation, std::string& error);
  bool read(HDF5Tool& file, Groups& groups, Entries& entries, std::string& error) const;
  bool write(HDF5Tool& file, const Groups& groups, const Entries& entries, std::string& error) const;
  const MaterialDefinitions& definitions;
  std::string libraryFilename;
  UserOptionsInterface* options = nullptr;
};
