#include "CalculiXMaterialsInterface.hpp"
#include "CoreMaterialsTreeData.hpp"
#include "CoreMaterials.hpp"
#include "CoreMaterialsLibrary.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::string CalculiXMaterialsInterface::get_material_name(int material_id)
{
  return ccx_core.mat->get_material_name(material_id);
}

std::string CalculiXMaterialsInterface::get_material_prefix(std::string material_name)
{
  return ccx_core.mat->get_material_prefix(material_name);
}

int CalculiXMaterialsInterface::get_group_property_size(std::string group_property)
{
  return ccx_core.mat->get_group_property_size(group_property);
}

bool CalculiXMaterialsInterface::check_material_exists(int material_id)
{
  return ccx_core.mat->check_material_exists(material_id);
}

std::string CalculiXMaterialsInterface::get_material_export_data(int topology_id,int iteration_id) // gets the export data from materials core
{
  return ccx_core.mat->get_material_export_data(topology_id, iteration_id);
}

bool CalculiXMaterialsInterface::create_materiallibrary_group(std::string name)
{
  return ccx_core.mat_library->create_group(name);
}

bool CalculiXMaterialsInterface::modify_materiallibrary_group(std::string name, std::string data, int mode)
{
  return ccx_core.mat_library->modify_group(name, data, mode);
}

bool CalculiXMaterialsInterface::delete_materiallibrary_group(std::string name)
{
  return ccx_core.mat_library->delete_group(name);
}

bool CalculiXMaterialsInterface::create_materiallibrary_material(std::string name, std::string groupname)
{
  return ccx_core.mat_library->create_material(name, groupname);
}

bool CalculiXMaterialsInterface::modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data)
{
  return ccx_core.mat_library->modify_material(name, groupname, data, mode, value_data);
}

bool CalculiXMaterialsInterface::delete_materiallibrary_material(std::string name, std::string groupname)
{
  return ccx_core.mat_library->delete_material(name, groupname);
}

bool CalculiXMaterialsInterface::export_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name)
{
  return ccx_core.mat_library->export_material(name, groupname, cubit_name);
}

bool CalculiXMaterialsInterface::import_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name)
{
  return ccx_core.mat_library->import_material(name, groupname, cubit_name);
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_material_tree_data()
{ 
  return CoreMaterialsTreeData::get_material_tree_data(*ccx_core.mat, *ccx_core.mat_iface);
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_material_group_properties()
{ 
  return ccx_core.mat->group_properties;
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_materiallibrary_tree_data()
{ 
  return ccx_core.mat_library->get_materiallibrary_tree_data();
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_materiallibrary_material_properties(std::string name, std::string group)
{ 
  return ccx_core.mat_library->get_materiallibrary_material_properties(name, group);
}

std::vector<std::vector<double>> CalculiXMaterialsInterface::get_materiallibrary_material_values(std::string name, std::string group, std::string property)
{
  return ccx_core.mat_library->get_materiallibrary_material_values(name, group, property);
}


bool CalculiXMaterialsInterface::get_material_properties(int id, MaterialProperties& properties, std::string& error)
{
  return ccx_core.mat->get_properties(id, properties, error);
}
