#include "Core/CalculiXCore.hpp"
#include "CoreMaterials.hpp"
#include "CoreMaterialsTreeData.hpp"
#include "CubitMessage.hpp"

std::string CalculiXCore::get_material_name(int material_id)
{
  return mat->get_material_name(material_id);
}

std::string CalculiXCore::get_material_prefix(std::string material_name)
{
  return mat->get_material_prefix(material_name);
}

bool CalculiXCore::check_material_exists(int material_id)
{
  return mat->check_material_exists(material_id);
}

std::string CalculiXCore::get_material_export_data(int topology_id, int iteration_id)
{
  return mat->get_material_export_data(topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_material_tree_data()
{
  return CoreMaterialsTreeData::get_material_tree_data(*mat, *mat_iface);
}

std::vector<std::vector<std::string>> CalculiXCore::get_material_group_properties()
{
  return mat->group_properties;
}

int CalculiXCore::get_group_property_size(std::string group_property)
{
  return mat->get_group_property_size(group_property);
}

bool CalculiXCore::check_topology_material_assignment(int topology_id, int iteration_id, int element_id)
{
  if (!mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return false;
  }

  return mat->check_topology_material_assignment(topology_id, iteration_id, element_id);
}

bool CalculiXCore::get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name)
{
  if (!mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return false;
  }

  return mat->get_topology_group_material_name(topology_id, iteration_id, element_id, material_name);
}

std::string CalculiXCore::get_topology_material_name(int topology_id, int element_id)
{
  if (!mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return "";
  }

  return mat->get_topology_material_name(topology_id, element_id);
}
