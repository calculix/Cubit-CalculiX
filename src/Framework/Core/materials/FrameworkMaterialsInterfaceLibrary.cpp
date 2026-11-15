#include "FrameworkMaterialsInterface.hpp"
#include "CoreFrameworkMaterialLibrary.hpp"
#include "CubitMessage.hpp"
namespace {
bool library_result(bool success, const std::string& error) {
  if (!success) {
    PRINT_ERROR("%s\n", error.c_str());
  }
  return success;
}
}
bool FrameworkMaterialsInterface::create_materiallibrary_group(const std::string& name) {
  std::string error;
  return library_result(framework_materials().library().create_group(name, error), error);
}
bool FrameworkMaterialsInterface::modify_materiallibrary_group(const std::string& name, const std::string& data, int mode) {
  std::string error;
  return library_result(framework_materials().library().modify_group(name, data, mode, error), error);
}
bool FrameworkMaterialsInterface::delete_materiallibrary_group(const std::string& name) {
  std::string error;
  return library_result(framework_materials().library().delete_group(name, error), error);
}
bool FrameworkMaterialsInterface::create_materiallibrary_material(const std::string& name, const std::string& group) {
  std::string error;
  return library_result(framework_materials().library().create_material(workspaceId, name, group, error), error);
}
bool FrameworkMaterialsInterface::modify_materiallibrary_material(const std::string& name, const std::string& group, const std::string& data, int mode, const std::vector<double>& values) {
  std::string error;
  return library_result(framework_materials().library().modify_material(workspaceId, name, group, data, mode, values, error), error);
}
bool FrameworkMaterialsInterface::delete_materiallibrary_material(const std::string& name, const std::string& group) {
  std::string error;
  return library_result(framework_materials().library().delete_material(workspaceId, name, group, error), error);
}
bool FrameworkMaterialsInterface::export_materiallibrary_material(const std::string& name, const std::string& group, const std::string& materialName) {
  std::string error;
  const int id = get_material(materialName);
  return library_result(framework_materials().library().export_material(workspaceId, id, name, group, error), error);
}
bool FrameworkMaterialsInterface::import_materiallibrary_material(const std::string& name, const std::string& group, const std::string& materialName) {
  std::string error;
  int id = 0;
  return library_result(framework_materials().library().import_material(workspaceId, name, group, materialName, id, error), error);
}
std::vector<std::vector<std::string>> FrameworkMaterialsInterface::get_materiallibrary_tree_data() const {
  std::vector<std::vector<std::string>> data;
  std::string error;
  library_result(framework_materials().library().tree_data(workspaceId, data, error), error);
  return data;
}
std::vector<std::vector<std::string>> FrameworkMaterialsInterface::get_materiallibrary_material_properties(const std::string& name, const std::string& group) const {
  MaterialData material;
  std::string error;
  if (!library_result(framework_materials().library().read_material(workspaceId, name, group, material, error), error)) {
    return {};
  }
  return get_group_properties();
}
std::vector<std::vector<double>> FrameworkMaterialsInterface::get_materiallibrary_material_values(const std::string& name, const std::string& group, const std::string& property) const {
  MaterialData material;
  std::string error;
  if (!library_result(framework_materials().library().read_material(workspaceId, name, group, material, error), error)) {
    return {};
  }
  const auto properties = get_group_properties();
  for (const auto& definition : properties) {
    if (definition.front() != property) {
      continue;
    }
    if (definition[1] == "1") {
      double value = 0.0;
      if (!library_result(read_scalar(material, property, value, error), error)) {
        return {};
      }
      return {{value}};
    }
    std::vector<std::vector<double>> rows;
    if (!library_result(read_table(material, property, rows, error), error)) {
      return {};
    }
    return rows;
  }
  PRINT_ERROR("Unknown material property: %s\n", property.c_str());
  return {};
}
