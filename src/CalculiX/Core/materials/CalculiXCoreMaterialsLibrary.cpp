#include "Core/CalculiXCore.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"

bool CalculiXCore::create_materiallibrary_group(std::string name)
{
  return FrameworkMaterialsInterface("calculix").create_materiallibrary_group(name);
}

bool CalculiXCore::modify_materiallibrary_group(std::string name, std::string data, int mode)
{
  return FrameworkMaterialsInterface("calculix").modify_materiallibrary_group(name, data, mode);
}

bool CalculiXCore::delete_materiallibrary_group(std::string name)
{
  return FrameworkMaterialsInterface("calculix").delete_materiallibrary_group(name);
}

bool CalculiXCore::create_materiallibrary_material(std::string name, std::string groupname)
{
  return FrameworkMaterialsInterface("calculix").create_materiallibrary_material(name, groupname);
}

bool CalculiXCore::modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data)
{
  return FrameworkMaterialsInterface("calculix").modify_materiallibrary_material(name, groupname, data, mode, value_data);
}

bool CalculiXCore::delete_materiallibrary_material(std::string name, std::string groupname)
{
  return FrameworkMaterialsInterface("calculix").delete_materiallibrary_material(name, groupname);
}

bool CalculiXCore::export_materiallibrary_material(std::string name, std::string groupname, std::string material_name)
{
  return FrameworkMaterialsInterface("calculix").export_materiallibrary_material(name, groupname, material_name);
}

bool CalculiXCore::import_materiallibrary_material(std::string name, std::string groupname, std::string material_name)
{
  return FrameworkMaterialsInterface("calculix").import_materiallibrary_material(name, groupname, material_name);
}

std::vector<std::vector<std::string>> CalculiXCore::get_materiallibrary_tree_data()
{
  return FrameworkMaterialsInterface("calculix").get_materiallibrary_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCore::get_materiallibrary_material_properties(std::string name, std::string group)
{
  return FrameworkMaterialsInterface("calculix").get_materiallibrary_material_properties(name, group);
}

std::vector<std::vector<double>> CalculiXCore::get_materiallibrary_material_values(std::string name, std::string group, std::string property)
{
  return FrameworkMaterialsInterface("calculix").get_materiallibrary_material_values(name, group, property);
}
