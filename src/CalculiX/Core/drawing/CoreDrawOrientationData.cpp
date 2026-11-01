#include "CoreDrawOrientationData.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/mesh/CoreMeshGeometry.hpp"
#include "ScientificNumber.hpp"

#include <string>

std::vector<std::vector<double>> CoreDrawOrientationData::get_draw_data_for_orientation(
  CoreOrientations& orientations, CoreSections& sections, int id)
{
  std::vector<std::vector<double>> draw_data;
  int orientation_data_id = orientations.get_orientations_data_id_from_orientation_id(id);

  if (orientation_data_id == -1)
  {
    return draw_data;
  }

  const auto& orientation = orientations.orientations_data[orientation_data_id];
  std::vector<double> orientation_data(3);
  std::vector<double> a_coord(3);
  std::vector<double> b_coord(3);

  orientation_data[0] = orientation[2];
  orientation_data[1] = orientation[6];

  const int rotation_index = orientations.get_rotation_data_id_from_rotation_id(orientation[7]);
  const int a_index = orientations.get_a_data_id_from_a_id(orientation[4]);
  const int b_index = orientations.get_b_data_id_from_b_id(orientation[5]);

  if (rotation_index == -1 || a_index == -1 || b_index == -1)
  {
    return draw_data;
  }

  orientation_data[2] = ScientificNumber::parse_with_precision(orientations.rotation_data[rotation_index][1]);

  for (int i = 0; i < 3; ++i)
  {
    a_coord[i] = ScientificNumber::parse_with_precision(orientations.a_data[a_index][i + 1]);
    b_coord[i] = ScientificNumber::parse_with_precision(orientations.b_data[b_index][i + 1]);
  }

  const auto linked_blocks = sections.get_orientation_block_ids(id);

  for (int block_id : linked_blocks)
  {
    std::vector<double> section_center = CoreMeshGeometry::get_block_bounding_box_center(block_id);

    if (!section_center.empty())
    {
      draw_data.push_back(orientation_data);
      draw_data.push_back(section_center);
      draw_data.push_back(a_coord);
      draw_data.push_back(b_coord);
    }
  }

  return draw_data;
}
