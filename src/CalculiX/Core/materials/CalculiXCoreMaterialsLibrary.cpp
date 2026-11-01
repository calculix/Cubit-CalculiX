#include "Core/CalculiXCore.hpp"
#include "CoreMaterialsLibrary.hpp"

bool CalculiXCore::create_materiallibrary_group(std::string name)
{
  return mat_library->create_group(name);
}

bool CalculiXCore::modify_materiallibrary_group(std::string name, std::string data, int mode)
{
  return mat_library->modify_group(name, data, mode);
}

bool CalculiXCore::delete_materiallibrary_group(std::string name)
{
  return mat_library->delete_group(name);
}

bool CalculiXCore::create_materiallibrary_material(std::string name, std::string groupname)
{
  return mat_library->create_material(name, groupname);
}

bool CalculiXCore::modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data)
{
  return mat_library->modify_material(name, groupname, data, mode, value_data);
}

bool CalculiXCore::delete_materiallibrary_material(std::string name, std::string groupname)
{
  return mat_library->delete_material(name, groupname);
}

bool CalculiXCore::export_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name)
{
  return mat_library->export_material(name, groupname, cubit_name);
}

bool CalculiXCore::import_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name)
{
  return mat_library->import_material(name, groupname, cubit_name);
}

std::vector<std::vector<std::string>> CalculiXCore::get_materiallibrary_tree_data()
{
  return mat_library->get_materiallibrary_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCore::get_materiallibrary_material_properties(std::string name, std::string group)
{
  return mat_library->get_materiallibrary_material_properties(name, group);
}

std::vector<std::vector<double>> CalculiXCore::get_materiallibrary_material_values(std::string name, std::string group, std::string property)
{
  return mat_library->get_materiallibrary_material_values(name, group, property);
}
