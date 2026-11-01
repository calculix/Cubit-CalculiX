#ifndef COREMATERIALSTREEDATA_HPP
#define COREMATERIALSTREEDATA_HPP

#include <string>
#include <vector>

class CoreMaterials;
class MaterialInterface;

class CoreMaterialsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_material_tree_data(
    CoreMaterials& materials, MaterialInterface& material_interface);
};

#endif
