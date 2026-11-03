#include "CoreDrawMeshGeometry.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace
{
bool append_connectivity(const std::string& entity, int id, bool wireframe, const std::string& color, DrawGeometry& geometry)
{
  // Cubit/Exodus corner order. Interior faces/edges do not need Cubit entities.
  std::vector<std::vector<std::size_t>> faces;
  std::size_t corners = 0;
  if (entity == "hex")
  {
    corners = 8;
    faces = {{0, 3, 2, 1}, {4, 5, 6, 7}, {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}};
  }
  else if (entity == "tet")
  {
    corners = 4;
    faces = {{0, 2, 1}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};
  }
  else if (entity == "wedge")
  {
    corners = 6;
    faces = {{0, 2, 1}, {3, 4, 5}, {0, 1, 4, 3}, {1, 2, 5, 4}, {2, 0, 3, 5}};
  }
  else if (entity == "pyramid")
  {
    corners = 5;
    faces = {{0, 3, 2, 1}, {0, 1, 4}, {1, 2, 4}, {2, 3, 4}, {3, 0, 4}};
  }
  const auto nodes = CubitInterface::get_connectivity(entity, id);
  if (corners == 0)
  {
    if (entity == "edge")
    {
      corners = nodes.size() == 1 ? 1 : 2;
    }
    else if (entity == "face" || entity == "tri" || entity == "quad")
    {
      if (nodes.size() == 3 || nodes.size() == 6 || nodes.size() == 7)
      {
        corners = 3;
      }
      else if (nodes.size() == 4 || nodes.size() == 8 || nodes.size() == 9)
      {
        corners = 4;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  if (nodes.size() < corners)
  {
    return false;
  }
  std::vector<std::vector<double>> points;
  std::set<int> uniqueNodes;
  for (std::size_t i = 0; i < corners; ++i)
  {
    if (nodes[i] <= 0 || !uniqueNodes.insert(nodes[i]).second)
    {
      return false;
    }
    const auto coordinate = CubitInterface::get_nodal_coordinates(nodes[i]);
    if (coordinate.size() < 3 || !std::isfinite(coordinate[0]) || !std::isfinite(coordinate[1]) || !std::isfinite(coordinate[2]))
    {
      return false;
    }
    points.push_back({coordinate[0], coordinate[1], coordinate[2]});
  }
  DrawGeometry result;
  if (corners == 1)
  {
    if (!result.point(points[0], color))
    {
      return false;
    }
  }
  else if (corners == 2)
  {
    if (!result.line(points[0], points[1], color))
    {
      return false;
    }
  }
  else
  {
    if (faces.empty())
    {
      std::vector<std::size_t> face;
      for (std::size_t i = 0; i < corners; ++i)
      {
        face.push_back(i);
      }
      faces.push_back(std::move(face));
    }
    std::set<std::pair<std::size_t, std::size_t>> edges;
    for (const auto& face : faces)
    {
      if (wireframe)
      {
        for (std::size_t i = 0; i < face.size(); ++i)
        {
          const auto edge = std::minmax(face[i], face[(i + 1) % face.size()]);
          if (edges.emplace(edge.first, edge.second).second && !result.line(points[edge.first], points[edge.second], color))
          {
            return false;
          }
        }
      }
      else
      {
        std::vector<std::vector<double>> polygon;
        for (std::size_t corner : face)
        {
          polygon.push_back(points[corner]);
        }
        if (!result.polygon(polygon, color))
        {
          return false;
        }
      }
    }
  }
  // Invalid connectivity never leaves a partially appended element.
  geometry.append(result);
  return true;
}
}

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
  return append_connectivity(entity, id, wireframe, color, geometry);
}
