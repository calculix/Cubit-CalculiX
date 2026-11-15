#include "CalculiXMaterialsInterface.hpp"
#include "CoreMaterialsTreeData.hpp"
#include "CoreMaterials.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include <string>
#include <vector>
#include "CubitMessage.hpp"
#include <cmath>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXMaterialsInterface::set_topology_iteration_factors(int topology_id, int iteration_id, const std::vector<int>& element_ids, const std::vector<double>& stiffness_factors, int material_cap, bool stable_levels, double minimum_factor)
{
  if (!ccx_core.mat)
  {
    PRINT_ERROR("Materials core is not initialized.\n");
    return false;
  }
  return ccx_core.mat->set_topology_iteration_factors(topology_id, iteration_id, element_ids, stiffness_factors, material_cap, stable_levels, minimum_factor);
}

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
  return FrameworkMaterialsInterface("calculix").create_materiallibrary_group(name);
}

bool CalculiXMaterialsInterface::modify_materiallibrary_group(std::string name, std::string data, int mode)
{
  return FrameworkMaterialsInterface("calculix").modify_materiallibrary_group(name, data, mode);
}

bool CalculiXMaterialsInterface::delete_materiallibrary_group(std::string name)
{
  return FrameworkMaterialsInterface("calculix").delete_materiallibrary_group(name);
}

bool CalculiXMaterialsInterface::create_materiallibrary_material(std::string name, std::string groupname)
{
  return FrameworkMaterialsInterface("calculix").create_materiallibrary_material(name, groupname);
}

bool CalculiXMaterialsInterface::modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data)
{
  return FrameworkMaterialsInterface("calculix").modify_materiallibrary_material(name, groupname, data, mode, value_data);
}

bool CalculiXMaterialsInterface::delete_materiallibrary_material(std::string name, std::string groupname)
{
  return FrameworkMaterialsInterface("calculix").delete_materiallibrary_material(name, groupname);
}

bool CalculiXMaterialsInterface::export_materiallibrary_material(std::string name, std::string groupname, std::string material_name)
{
  return FrameworkMaterialsInterface("calculix").export_materiallibrary_material(name, groupname, material_name);
}

bool CalculiXMaterialsInterface::import_materiallibrary_material(std::string name, std::string groupname, std::string material_name)
{
  return FrameworkMaterialsInterface("calculix").import_materiallibrary_material(name, groupname, material_name);
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_material_tree_data()
{ 
  return CoreMaterialsTreeData::get_material_tree_data(*ccx_core.mat, *ccx_core.mat->mat_iface);
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_material_group_properties()
{ 
  return ccx_core.mat->material_api.get_group_properties();
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_materiallibrary_tree_data()
{ 
  return FrameworkMaterialsInterface("calculix").get_materiallibrary_tree_data();
}

std::vector<std::vector<std::string>> CalculiXMaterialsInterface::get_materiallibrary_material_properties(std::string name, std::string group)
{ 
  return FrameworkMaterialsInterface("calculix").get_materiallibrary_material_properties(name, group);
}

std::vector<std::vector<double>> CalculiXMaterialsInterface::get_materiallibrary_material_values(std::string name, std::string group, std::string property)
{
  return FrameworkMaterialsInterface("calculix").get_materiallibrary_material_values(name, group, property);
}


bool CalculiXMaterialsInterface::get_material_properties(int id, MaterialProperties& properties, std::string& error)
{
  return ccx_core.mat->get_properties(id, properties, error);
}

bool CalculiXMaterialsInterface::get_topology_iteration_factors(int topology_id, int iteration_id, std::map<int, double>& factors)
{
  factors.clear();
  if (!ccx_core.mat || topology_id <= 0 || iteration_id <= 0)
  {
    PRINT_ERROR("Invalid topology material factor query.\n");
    return false;
  }
  const auto collection = ccx_core.mat->topology_material_data.find(topology_id);
  if (collection == ccx_core.mat->topology_material_data.end())
  {
    PRINT_ERROR("Topology material collection is missing.\n");
    return false;
  }
  const auto iteration = collection->second.iteration_factors.find(iteration_id);
  if (iteration == collection->second.iteration_factors.end() || iteration->second.empty() || iteration->second.size() != collection->second.element_base_material.size())
  {
    PRINT_ERROR("Topology applied factors are missing or incomplete.\n");
    return false;
  }
  for (const auto& factor : iteration->second)
  {
    if (collection->second.element_base_material.count(factor.first) == 0 || !std::isfinite(factor.second) || factor.second <= 0.0 || factor.second > 1.0)
    {
      PRINT_ERROR("Invalid stored topology applied factor.\n");
      return false;
    }
  }
  factors = iteration->second;
  return true;
}
