#ifndef COREMESHTREEDATA_HPP
#define COREMESHTREEDATA_HPP

#include <string>
#include <vector>

class CoreBlocks;
class MeshExportInterface;

class CoreMeshTreeData
{
public:
  static std::vector<std::vector<std::string>> get_blocks_tree_data(CoreBlocks& blocks, MeshExportInterface& mesh);
  static std::vector<std::vector<std::string>> get_nodeset_tree_data(MeshExportInterface& mesh);
  static std::vector<std::vector<std::string>> get_sideset_tree_data(MeshExportInterface& mesh);
};

#endif
