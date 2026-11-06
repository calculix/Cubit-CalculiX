#include "CoreDrawMeshGeometry.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Framework/Drawing/DrawingMeshGeometry.hpp"
#include "CubitInterface.hpp"

bool CoreDrawMeshGeometry::append(const std::string& requested, int id, bool wireframe, const std::string& color, DrawingGeometry& geometry)
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
  return DrawingMeshGeometry::append(entity, id, wireframe, color, geometry);
}
