#include "CoreMaterialsTreeData.hpp"
#include "CoreMaterials.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"

std::vector<std::vector<std::string>> CoreMaterialsTreeData::get_material_tree_data(CoreMaterials& materials, FrameworkMaterialsInterface& material_interface)
{
  std::vector<std::vector<std::string>> tree_data;
  for (int id : material_interface.get_material_ids()) {
    tree_data.push_back({std::to_string(id), material_interface.get_material_name(id)});
  }
  return tree_data;
}
