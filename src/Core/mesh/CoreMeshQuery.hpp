#ifndef COREMESHQUERY_HPP
#define COREMESHQUERY_HPP

#include <string>
#include <vector>

class MeshExportInterface;
class CoreBlocks;

class CoreMeshQuery
{
public:
  explicit CoreMeshQuery(MeshExportInterface& mesh);

  std::string get_block_name(int block_id);
  std::vector<int> get_block_node_ids(int block_id);
  std::vector<int> get_block_element_ids(CoreBlocks& blocks, int block_id);
  std::vector<std::vector<int>> get_element_id_type_connectivity(CoreBlocks& blocks);
  std::string get_nodeset_name(int nodeset_id);
  std::string get_sideset_name(int sideset_id);
  bool check_nodeset_exists(int nodeset_id);
  bool check_sideset_exists(int sideset_id);
  static bool check_global_element_exists(int element_id);
  static bool check_vertex_exists(int vertex_id);
  static bool check_curve_exists(int curve_id);
  static bool check_surface_exists(int surface_id);
  static bool check_vertex_in_nodeset_exists(int vertex_id, int nodeset_id);

private:
  MeshExportInterface& mesh;
};

#endif
