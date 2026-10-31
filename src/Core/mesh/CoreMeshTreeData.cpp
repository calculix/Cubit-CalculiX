#include "CoreMeshTreeData.hpp"
#include "CoreBlocks.hpp"
#include "CubitInterface.hpp"
#include "MeshExportInterface.hpp"

std::vector<std::vector<std::string>> CoreMeshTreeData::get_blocks_tree_data(
  CoreBlocks& blocks, MeshExportInterface& mesh)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& block_data : blocks.blocks_data)
  {
    int block_id = block_data[0];
    BlockHandle block;
    mesh.get_block_handle(block_id, block);
    std::string name = mesh.name_from_handle(block);

    if (name.empty())
    {
      name = "Block_" + std::to_string(block_id);
    }

    tree_data.push_back({std::to_string(block_id), name, blocks.get_ccx_element_type_name(block_id)});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreMeshTreeData::get_nodeset_tree_data(MeshExportInterface& mesh)
{
  std::vector<std::vector<std::string>> tree_data;
  std::vector<int> nodeset_ids = CubitInterface::get_nodeset_id_list();

  for (int nodeset_id : nodeset_ids)
  {
    NodesetHandle nodeset;
    mesh.get_nodeset_handle(nodeset_id, nodeset);
    std::string name = mesh.get_nodeset_name(nodeset);
    int id = mesh.id_from_handle(nodeset);

    if (name.empty())
    {
      name = "Nodeset_" + std::to_string(id);
    }

    tree_data.push_back({std::to_string(id), name});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreMeshTreeData::get_sideset_tree_data(MeshExportInterface& mesh)
{
  std::vector<std::vector<std::string>> tree_data;
  std::vector<int> sideset_ids = CubitInterface::get_sideset_id_list();

  for (int sideset_id : sideset_ids)
  {
    SidesetHandle sideset;
    mesh.get_sideset_handle(sideset_id, sideset);
    std::string name = mesh.get_sideset_name(sideset);
    int id = mesh.id_from_handle(sideset);

    if (name.empty())
    {
      name = "Sideset_" + std::to_string(id);
    }

    tree_data.push_back({std::to_string(id), name});
  }

  return tree_data;
}
