#include "CoreConnectionEndpointResolver.hpp"
#include "CoreConnections.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitInterface.hpp"
#include <QString>

CoreConnectionEndpointResolver::CoreConnectionEndpointResolver(const CoreConnections& connections,
  const std::set<int>& structural, const std::map<int,std::array<int,2>>& rigid_roles,
  const std::map<int,Position>& rigid_positions, const SimUnits::QuantityUnit& length_quantity)
  : model(connections), mesh_nodes(structural), roles(rigid_roles), positions(rigid_positions), length(length_quantity)
{
  for (const auto& item : roles)
  {
    rotations.insert(item.second[1]);
  }
}

bool CoreConnectionEndpointResolver::resolve(const ConnectionEndpoint& endpoint, Resolved& result, std::string& error) const
{
  result = {};
  int vertex = endpoint.id;
  if (endpoint.kind == ConnectionEndpointKind::ReferencePoint)
  {
    const auto point = model.points().find(endpoint.id);
    if (point == model.points().end())
    {
      error = "Unknown reference point " + std::to_string(endpoint.id) + ".";
      return false;
    }
    const auto role = roles.find(endpoint.id);
    if (role != roles.end())
    {
      result.first = role->second[0];
      result.second = positions.at(endpoint.id);
    }
    else
    {
      for (const auto& item : model.attachments())
      {
        if (item.second.point == endpoint.id)
        {
          error = "Distributing RP " + point->second.name + " cannot provide physical endpoint motion.";
          return false;
        }
      }
      if (point->second.geometry != "vertex")
      {
        error = "RP " + point->second.name + " needs a rigid attachment or a meshed vertex.";
        return false;
      }
      vertex = point->second.entity;
    }
  }
  if (result.first == 0)
  {
    result.first = CubitInterface::get_vertex_node(vertex);
    if (result.first <= 0 || !mesh_nodes.count(result.first) || rotations.count(result.first))
    {
      error = "Vertex " + std::to_string(vertex) + " needs an exported translational node.";
      return false;
    }
  }
  if (mesh_nodes.count(result.first))
  {
    result.second = CubitInterface::get_nodal_coordinates(result.first);
  }
  for (auto& coordinate : result.second)
  {
    double si = 0.0;
    if (!framework_units().geometry_to_si(coordinate, si, error) ||
        !framework_units().to_solver(si, length.dimension, "calculix", coordinate, error))
    {
      return false;
    }
  }
  // Preserve the coordinates written to the deck for result linking and reference length.
  for (auto& coordinate : result.second)
  {
    coordinate = QString::number(coordinate, 'e', 12).toDouble();
  }
  return true;
}
