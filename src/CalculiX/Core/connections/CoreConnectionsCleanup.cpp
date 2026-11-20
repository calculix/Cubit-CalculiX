#include "CoreConnectionsCleanup.hpp"
#include "CoreConnections.hpp"
#include "CubitInterface.hpp"
#include <algorithm>
#include <set>
#include <vector>

bool CoreConnectionsCleanup::cleanup(CoreConnections& connections, std::string& log)
{
  std::set<int> points;
  std::set<int> bodies;
  for (const auto& entry : connections.points())
  {
    const auto& point = entry.second;
    if ((point.geometry != "xyz" && CubitInterface::parse_cubit_list(point.geometry, std::to_string(point.entity)).empty()) ||
        (point.rotation_vertex > 0 && CubitInterface::parse_cubit_list("vertex", std::to_string(point.rotation_vertex)).empty()))
    {
      points.insert(point.id);
    }
  }
  for (const auto& entry : connections.bodies())
  {
    const auto& body = entry.second;
    if (points.count(body.point) || CubitInterface::parse_cubit_list(body.geometry, std::to_string(body.entity)).empty())
    {
      bodies.insert(body.id);
    }
  }
  std::set<int> invalid_endpoints = points;
  for (int id : bodies)
  {
    invalid_endpoints.insert(connections.bodies().at(id).point);
  }
  std::set<int> joints;
  for (const auto& entry : connections.joints())
  {
    if (invalid_endpoints.count(entry.second.point_a) || invalid_endpoints.count(entry.second.point_b))
    {
      joints.insert(entry.first);
    }
  }
  for (int id : joints)
  {
    const auto& joint = connections.joints().at(id);
    if (joint.owns_endpoints)
    {
      for (int endpoint : {joint.point_a, joint.point_b})
      {
        const auto shared = std::any_of(connections.joints().begin(), connections.joints().end(), [&](const auto& entry)
        {
          return !joints.count(entry.first) && (entry.second.point_a == endpoint || entry.second.point_b == endpoint);
        });
        if (!shared)
        {
          invalid_endpoints.insert(endpoint);
        }
      }
    }
  }
  const auto lost = [&](const ConnectionEndpoint& endpoint)
  {
    if (endpoint.kind == ConnectionEndpointKind::ReferencePoint)
    {
      return invalid_endpoints.count(endpoint.id) != 0;
    }
    return CubitInterface::parse_cubit_list("vertex", std::to_string(endpoint.id)).empty();
  };
  std::vector<int> masses;
  std::vector<int> springs;
  for (const auto& entry : connections.point_masses())
  {
    if (lost(entry.second.endpoint))
    {
      masses.push_back(entry.first);
    }
  }
  for (const auto& entry : connections.springs())
  {
    if (lost(entry.second.first) || (entry.second.second && lost(*entry.second.second)))
    {
      springs.push_back(entry.first);
    }
  }
  for (int id : masses)
  {
    connections.remove_point_mass(id);
    log += "Deleted point mass " + std::to_string(id) + " with missing endpoint.\n";
  }
  for (int id : springs)
  {
    connections.remove_spring(id);
    log += "Deleted spring " + std::to_string(id) + " with missing endpoint.\n";
  }
  for (int id : joints)
  {
    connections.remove_joint(id);
    log += "Deleted joint " + std::to_string(id) + " with missing endpoint.\n";
  }
  for (int id : bodies)
  {
    connections.remove_body(id);
    log += "Deleted rigid body " + std::to_string(id) + " with missing attachment or reference point.\n";
  }
  std::set<int> attachments;
  for (const auto& entry : connections.attachments())
  {
    const auto& a = entry.second;
    if (points.count(a.point) || CubitInterface::parse_cubit_list(a.geometry, std::to_string(a.entity)).empty())
    {
      attachments.insert(a.id);
    }
  }
  for (int id : attachments)
  {
    connections.remove_attachment(id);
    log += "Deleted distributing attachment " + std::to_string(id) + " with missing geometry or RP.\n";
  }
  for (int id : points)
  {
    connections.remove_point(id);
    log += "Deleted reference point " + std::to_string(id) + " with missing placement geometry.\n";
  }
  return !masses.empty() || !springs.empty() || !attachments.empty() || !points.empty() || !bodies.empty();
}
