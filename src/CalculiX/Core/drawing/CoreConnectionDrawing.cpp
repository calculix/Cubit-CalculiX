#include "CoreConnectionDrawing.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "Framework/Drawing/DrawingSymbols.hpp"
#include "Core/connections/CoreJointGeometry.hpp"
#include "CubitInterface.hpp"
#include <cmath>

namespace
{
bool endpoint_position(const ConnectionEndpoint& endpoint, std::vector<double>& output)
{
  std::array<double,3> position;
  if (endpoint.kind == ConnectionEndpointKind::ReferencePoint)
  {
    std::string error;
    if (!CalculiXConnectionsInterface().point_position(endpoint.id,position,error))
    {
      return false;
    }
  }
  else
  {
    if (CubitInterface::parse_cubit_list("vertex",std::to_string(endpoint.id)).empty())
    {
      return false;
    }
    position = CubitInterface::get_center_point("vertex",endpoint.id);
  }
  output.assign(position.begin(),position.end());
  return true;
}
}

bool CoreConnectionDrawing::resolve(const CoreConnections& connections, DrawEntityKey key, double size, DrawingGeometry& geometry)
{
  if (key.feature == DrawFeature::PointMass)
  {
    const auto found = connections.point_masses().find(key.id);
    std::vector<double> endpoint;
    return found != connections.point_masses().end() && endpoint_position(found->second.endpoint, endpoint) &&
      DrawingSymbols::pointMass(geometry, endpoint, size);
  }
  if (key.feature == DrawFeature::Spring)
  {
    const auto found = connections.springs().find(key.id);
    std::vector<double> first, second;
    if (found == connections.springs().end() || !endpoint_position(found->second.first, first))
    {
      return false;
    }
    const auto& spring = found->second;
    if (spring.second)
    {
      if (!endpoint_position(*spring.second, second))
      {
        return false;
      }
    }
    else
    {
      second = first;
      second.at(spring.direction - 1) -= 3.0 * size;
    }
    return DrawingSymbols::spring(geometry, first, second, !spring.second, size);
  }
  int point_id = key.id;
  const ConnectionRigidBody* body = nullptr;
  const ConnectionJoint* joint = nullptr;
  if (key.feature == DrawFeature::RigidBody)
  {
    const auto found = connections.bodies().find(key.id);
    if (found == connections.bodies().end())
    {
      return false;
    }
    body = &found->second;
    point_id = body->point;
  }
  else if (key.feature == DrawFeature::Joint)
  {
    const auto found = connections.joints().find(key.id);
    if (found == connections.joints().end())
    {
      return false;
    }
    joint = &found->second;
    point_id = joint->point_a;
  }
  auto found = connections.points().find(point_id);
  if (found == connections.points().end())
  {
    return false;
  }
  const auto& point = found->second;
  auto position = point.position;
  if (point.geometry != "xyz")
  {
    if (CubitInterface::parse_cubit_list(point.geometry,std::to_string(point.entity)).empty())
    {
      return false;
    }
    position = CubitInterface::get_center_point(point.geometry,point.entity);
  }
  auto axis = joint ? joint->axis : std::array<double,3>{0,0,1};
  for (const auto& entry : connections.joints())
  {
    const auto& item = entry.second;
    if (item.coaxial && (item.point_a == point_id || item.point_b == point_id))
    {
      std::array<double,3> direction;
      std::string error;
      if (!CoreJointGeometry::coaxial(connections.points().at(item.point_a).entity,connections.points().at(item.point_b).entity,position,direction,error))
      {
        return false;
      }
      if (joint && joint->id == item.id)
      {
        axis = direction;
      }
    }
  }
  const std::vector<double> center(position.begin(),position.end());
  if (!geometry.point(center,"red"))
  {
    return false;
  }
  if (joint)
  {
    const double norm = std::hypot(axis[0],axis[1],axis[2]);
    auto start = center;
    auto end = center;
    for (int i = 0; i < 3; ++i)
    {
      start[i] -= size * axis[i] / norm;
      end[i] += size * axis[i] / norm;
    }
    return geometry.line(start,end,"yellow");
  }
  if (body)
  {
    const auto members = CubitInterface::parse_cubit_list("node","in " + body->geometry + " " + std::to_string(body->entity));
    for (int id : members)
    {
      const auto coordinates = CubitInterface::get_nodal_coordinates(id);
      if (!geometry.point({coordinates[0],coordinates[1],coordinates[2]},"yellow"))
      {
        return false;
      }
    }
  }
  return true;
}
