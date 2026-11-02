#include "CoreDrawMeshGeometry.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include <set>

bool CoreDrawMeshGeometry::append(const std::string& requested, int id, bool wireframe, const std::string& color, DrawGeometry& geometry)
{
  if (id <= 0)
  {
    return false;
  }
  CalculiXMeshInterface mesh;
  if (requested == "block")
  {
    if (!mesh.check_block_exists(id))
    {
      return false;
    }
    for (int element : mesh.get_block_element_ids(id))
    {
      if (!append("element", element, wireframe, color, geometry))
      {
        return false;
      }
    }
    return true;
  }
  std::string entity = requested;
  if (entity == "element")
  {
    entity = CubitInterface::get_element_type(id);
    if (entity != "hex" && entity != "tet" && entity != "wedge" && entity != "pyramid" && entity != "face" && entity != "edge")
    {
      entity = mesh.get_cubit_element_type_entity(entity);
    }
    id = CubitInterface::get_element_type_id(id);
    if (id <= 0 || entity.empty())
    {
      return false;
    }
  }
  const bool solid = entity == "hex" || entity == "tet" || entity == "wedge" || entity == "pyramid";
  if (solid || (wireframe && entity != "edge"))
  {
    const auto parts = CubitInterface::get_sub_elements(entity, id, wireframe ? 1 : 2);
    if (parts.empty())
    {
      return false;
    }
    const std::set<int> unique(parts.begin(), parts.end());
    for (int part : unique)
    {
      if (!append(wireframe ? "edge" : "face", part, wireframe, color, geometry))
      {
        return false;
      }
    }
    return true;
  }
  const auto nodes = CubitInterface::get_connectivity(entity, id);
  std::vector<std::vector<double>> points;
  for (int node : nodes)
  {
    const auto coordinate = CubitInterface::get_nodal_coordinates(node);
    points.push_back({coordinate[0], coordinate[1], coordinate[2]});
  }
  if (points.size() == 1)
  {
    return geometry.point(points[0], color);
  }
  if (points.size() == 2)
  {
    return geometry.line(points[0], points[1], color);
  }
  return geometry.polygon(points, color);
}
