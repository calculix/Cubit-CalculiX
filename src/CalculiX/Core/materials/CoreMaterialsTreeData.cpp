#include "CoreMaterialsTreeData.hpp"
#include "CoreMaterials.hpp"
#include "CubitInterface.hpp"
#include "MaterialInterface.hpp"

#include <algorithm>

std::vector<std::vector<std::string>> CoreMaterialsTreeData::get_material_tree_data(
  CoreMaterials& materials, MaterialInterface& material_interface)
{
  std::vector<std::vector<std::string>> tree_data;
  std::vector<std::string> material_names = CubitInterface::get_material_name_list();

  for (const auto& name : material_names)
  {
    MaterialInterface::Material material = material_interface.get_material(name);
    MaterialInterface::PropertyGroup group = material_interface.get_material_property_group(material);
    std::string group_name = material_interface.get_group_name(group);
    std::vector<std::string> groups = materials.get_group_list();

    if (std::find(groups.begin(), groups.end(), group_name) != groups.end())
    {
      tree_data.push_back({std::to_string(material_interface.get_material_id(material)), name});
    }
  }

  return tree_data;
}
