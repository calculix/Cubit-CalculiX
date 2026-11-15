#include "ImportMaterialCommand.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterials.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp"
#include "CubitMessage.hpp"
#include <exception>
ImportMaterialCommand::ImportMaterialCommand() = default;
std::vector<std::string> ImportMaterialCommand::get_syntax() {
  return {"sim import materiallibrary name <string:type='quoted',number='1',label='name'> groupname <string:type='quoted',number='1',label='groupname'> workspace <string:type='unquoted',number='1',label='workspace'> material_name <string:type='quoted',number='1',label='material_name'>"};
}
std::vector<std::string> ImportMaterialCommand::get_syntax_help() {
  return {"sim import materiallibrary name \"S235\" groupname \"Metals/Steel\" workspace calculix material_name \"Steel\""};
}
std::vector<std::string> ImportMaterialCommand::get_help() {
  return {"Copy a shared library material into a workspace with a new independent local ID. Later library changes do not change the imported material."};
}
bool ImportMaterialCommand::execute(CubitCommandData& data) {
  try {
    std::string name;
    std::string group;
    std::string workspace;
    std::string localName;
    std::string error;
    int id = 0;
    if (!data.get_string("name", name) || !data.get_string("groupname", group) || !data.get_string("workspace", workspace) || !data.get_string("material_name", localName)) {
      PRINT_ERROR("Material import requires a library name, group, workspace and local material name.\n");
      return false;
    }
    if (workspace != "calculix") {
      PRINT_ERROR("Workspace %s has no installed solver material schema.\n", workspace.c_str());
      return false;
    }
    if (!framework_materials().library().import_material(workspace, name, group, localName, id, error)) {
      PRINT_ERROR("Cannot import library material: %s\n", error.c_str());
      return false;
    }
    PRINT_INFO("Imported %s material %d: %s\n", workspace.c_str(), id, localName.c_str());
    return true;
  } catch (const std::exception& exception) {
    PRINT_ERROR("Cannot import library material: %s\n", exception.what());
  } catch (...) {
    PRINT_ERROR("Cannot import library material: unknown error.\n");
  }
  return false;
}
