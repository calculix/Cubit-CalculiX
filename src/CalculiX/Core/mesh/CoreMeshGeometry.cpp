#include "CoreMeshGeometry.hpp"
#include "CubitInterface.hpp"

#include <array>
#include <string>

std::vector<std::vector<double>> CoreMeshGeometry::get_nodeset_entities_coords(int nodeset_id)
{
  std::vector<std::vector<double>> coords;
  std::vector<int> node_ids;
  std::vector<int> volume_ids;
  std::vector<int> surface_ids;
  std::vector<int> curve_ids;
  std::vector<int> vertex_ids;

  CubitInterface::get_nodeset_children(nodeset_id, node_ids, volume_ids, surface_ids, curve_ids, vertex_ids);

  for (int node_id : node_ids)
  {
    std::array<double, 3> coord = CubitInterface::get_nodal_coordinates(node_id);
    coords.push_back({coord[0], coord[1], coord[2]});
  }

  for (int volume_id : volume_ids)
  {
    std::array<double, 3> coord = CubitInterface::get_center_point("volume", volume_id);
    coords.push_back({coord[0], coord[1], coord[2]});
  }

  for (int surface_id : surface_ids)
  {
    std::array<double, 3> coord = CubitInterface::get_center_point("surface", surface_id);
    coords.push_back({coord[0], coord[1], coord[2]});
  }

  for (int curve_id : curve_ids)
  {
    std::array<double, 3> coord = CubitInterface::get_center_point("curve", curve_id);
    coords.push_back({coord[0], coord[1], coord[2]});
  }

  for (int vertex_id : vertex_ids)
  {
    std::array<double, 3> coord = CubitInterface::get_center_point("vertex", vertex_id);
    coords.push_back({coord[0], coord[1], coord[2]});
  }

  return coords;
}

std::vector<double> CoreMeshGeometry::get_block_bounding_box_center(int block_id)
{
  std::vector< int > returned_node_list;
  std::vector< int > returned_sphere_list;
  std::vector< int > returned_edge_list;
  std::vector< int > returned_tri_list;
  std::vector< int > returned_face_list;
  std::vector< int > returned_pyramid_list;
  std::vector< int > returned_wedge_list;
  std::vector< int > returned_tet_list;
  std::vector< int > returned_hex_list;
  std::vector< int > returned_group_list;
  std::vector< int > returned_volume_list;
  std::vector< int > returned_surface_list;
  std::vector< int > returned_curve_list;
  std::vector< int > returned_vertex_list;
  std::vector<double> coord;

  CubitInterface::get_block_children(block_id,
   returned_group_list,
   returned_node_list,
   returned_sphere_list,
   returned_edge_list,
   returned_tri_list,
   returned_face_list,
   returned_pyramid_list,
   returned_tet_list,
   returned_hex_list,
   returned_wedge_list,
   returned_volume_list,
   returned_surface_list,
   returned_curve_list,
   returned_vertex_list
  );

  std::array<double,10> vector_list;
  std::vector<int> ids = CubitInterface::parse_cubit_list("vertex", "all");
  if (returned_volume_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("volume", returned_volume_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }
  else if (returned_surface_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("surface", returned_surface_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }
  else if (returned_curve_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("curve", returned_curve_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }
  else if (returned_vertex_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("vertex", returned_vertex_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }

  return coord;
}

std::vector<std::vector<double>> CoreMeshGeometry::get_sideset_entities_coords(int sideset_id)
{
  std::vector<std::vector<double>> tmp;

  std::vector< int > returned_face_list;
	std::vector< int > returned_surface_list;
	std::vector< int > returned_curve_list;

  CubitInterface::get_sideset_children(sideset_id,returned_face_list,returned_surface_list,returned_curve_list);

  for (size_t i = 0; i < returned_face_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("face",returned_face_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  for (size_t i = 0; i < returned_surface_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("surface",returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  for (size_t i = 0; i < returned_curve_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("curve",returned_curve_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  return tmp;
}

std::vector<std::vector<double>> CoreMeshGeometry::get_sideset_entities_coords_normals(int sideset_id)
{
  std::vector<std::vector<double>> tmp;

  std::vector< int > returned_face_list;
	std::vector< int > returned_surface_list;
	std::vector< int > returned_curve_list;

  CubitInterface::get_sideset_children(sideset_id,returned_face_list,returned_surface_list,returned_curve_list);

  /*
  for (size_t i = 0; i < returned_face_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("face",returned_face_list[i]);
    std::array<double,3> normal = CubitInterface::get_surface_normal_at_coord(-1,coord);
    if (!((normal[0]==0)&&(normal[1]==0)&&(normal[2]==0)))
    {
      tmp.push_back({coord[0],coord[1],coord[2],normal[0],normal[1],normal[2]});
    }
  }
  */

  for (size_t i = 0; i < returned_surface_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("surface",returned_surface_list[i]);
    std::array<double,3> normal = CubitInterface::get_surface_normal(returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2],normal[0],normal[1],normal[2]});
  }

  /*
  for (size_t i = 0; i < returned_curve_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("curve",returned_curve_list[i]);
    std::array<double,3> normal = CubitInterface::get_surface_normal(returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2],normal[0],normal[1],normal[2]});
  }
  */

  return tmp;
}

