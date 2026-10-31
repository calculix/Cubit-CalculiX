#ifndef COREMESHGEOMETRY_HPP
#define COREMESHGEOMETRY_HPP

#include <vector>

class CoreMeshGeometry
{
public:
  static std::vector<double> get_block_bounding_box_center(int block_id);
  static std::vector<std::vector<double>> get_nodeset_entities_coords(int nodeset_id);
  static std::vector<std::vector<double>> get_sideset_entities_coords(int sideset_id);
  static std::vector<std::vector<double>> get_sideset_entities_coords_normals(int sideset_id);
};

#endif
