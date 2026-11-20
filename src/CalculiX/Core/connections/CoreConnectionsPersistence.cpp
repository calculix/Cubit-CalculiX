#include "CoreConnectionsPersistence.hpp"
#include "CoreConnections.hpp"
#include "Core/application/CalculiXModelSchema.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <QString>
#include <utility>

namespace
{
const auto group = CalculiXModelSchema::path("Connections");
using Rows = std::vector<std::vector<std::string>>;

bool read_rows(HDF5Tool& file, const std::string& key, Rows& rows)
{
  return !file.nameExists(group + "/" + key) || file.read_dataset_string_rank_2(key, group, rows);
}

bool read_endpoint(const std::string& kind, const std::string& id, ConnectionEndpoint& endpoint)
{
  if (kind != "vertex" && kind != "rp")
  {
    return false;
  }
  bool ok = false;
  endpoint.id = QString::fromStdString(id).toInt(&ok);
  endpoint.kind = kind == "vertex" ? ConnectionEndpointKind::Vertex : ConnectionEndpointKind::ReferencePoint;
  return ok && endpoint.id > 0;
}

int number(const std::string& text)
{
  bool ok = false;
  int value = QString::fromStdString(text).toInt(&ok);
  return ok ? value : -1;
}
}

bool CoreConnectionsPersistence::read(HDF5Tool& file, CoreConnections& connections)
{
  Rows points;
  Rows bodies;
  Rows joints;
  if (!read_rows(file, "points", points) || !read_rows(file, "rigid_bodies", bodies) || !read_rows(file, "joints", joints))
  {
    PRINT_ERROR("Cannot read Connections data.\n");
    return false;
  }
  CoreConnections result;
  for (const auto& row : points)
  {
    if (row.size() != 9)
    {
      PRINT_ERROR("Invalid reference point record.\n");
      return false;
    }
    ConnectionPoint point{number(row[0]), row[1], row[2], number(row[3])};
    for (size_t i = 0; i < point.position.size(); ++i)
    {
      bool ok = false;
      point.position[i] = QString::fromStdString(row[4 + i]).toDouble(&ok);
      if (!ok)
      {
        PRINT_ERROR("Invalid reference point coordinates.\n");
        return false;
      }
    }
    point.rotation_vertex = number(row[7]);
    bool valid_step = false;
    point.history_step = QString::fromStdString(row[8]).toInt(&valid_step);
    if (!valid_step)
    {
      return false;
    }
    if (!result.add(point))
    {
      PRINT_ERROR("Invalid or duplicate reference point.\n");
      return false;
    }
  }
  for (const auto& row : bodies)
  {
    if (row.size() != 6 || (row[5] != "0" && row[5] != "1") ||
        !result.add(ConnectionRigidBody{number(row[0]), row[1], row[2], number(row[3]), number(row[4]), row[5] == "1"}))
    {
      PRINT_ERROR("Invalid rigid body or reference point assignment.\n");
      return false;
    }
  }
  for (const auto& row : joints)
  {
    if (row.size() != 11 || (row[8] != "0" && row[8] != "1") || (row[9] != "0" && row[9] != "1"))
    {
      PRINT_ERROR("Invalid joint record.\n");
      return false;
    }
    ConnectionJoint joint{number(row[0]), row[1], row[2], number(row[3]), number(row[4])};
    for (size_t i = 0; i < 3; ++i)
    {
      bool ok = false;
      joint.axis[i] = QString::fromStdString(row[5 + i]).toDouble(&ok);
      if (!ok)
      {
        PRINT_ERROR("Invalid joint axis.\n");
        return false;
      }
    }
    joint.coaxial = row[8] == "1";
    joint.owns_endpoints = row[9] == "1";
    bool valid_step = false;
    joint.history_step = QString::fromStdString(row[10]).toInt(&valid_step);
    if (!valid_step || !result.add(joint))
    {
      PRINT_ERROR("Invalid joint definition or endpoint reference.\n");
      return false;
    }
  }
  Rows attachments;
  if (!read_rows(file, "distributing", attachments))
  {
    return false;
  }
  for (const auto& row : attachments)
  {
    if (row.size() != 9 || !result.add(DistributingAttachment{number(row[0]),row[1],row[2],number(row[3]),
        number(row[4]),number(row[5]),row[6]=="1",row[7]=="1",row[8]=="1"}))
    {
      PRINT_ERROR("Invalid distributing attachment.\n");
      return false;
    }
  }
  Rows masses;
  Rows springs;
  if (!read_rows(file, "point_masses", masses) || !read_rows(file, "springs", springs))
  {
    PRINT_ERROR("Cannot read point masses or springs.\n");
    return false;
  }
  std::string error;
  for (const auto& row : masses)
  {
    ConnectionPointMass mass;
    if (row.size() != 6 || !read_endpoint(row[2], row[3], mass.endpoint))
    {
      PRINT_ERROR("Invalid point mass record.\n");
      return false;
    }
    mass.id = number(row[0]);
    mass.name = row[1];
    bool valid_step = false;
    mass.mass_si = QString::fromStdString(row[4]).toDouble();
    mass.history_step = QString::fromStdString(row[5]).toInt(&valid_step);
    if (!valid_step || !result.save(mass, false, error))
    {
      PRINT_ERROR("Cannot restore point mass %d: %s\n", mass.id, error.c_str());
      return false;
    }
  }
  for (const auto& row : springs)
  {
    ConnectionSpring spring;
    if (row.size() != 9 || !read_endpoint(row[2], row[3], spring.first))
    {
      PRINT_ERROR("Invalid spring record.\n");
      return false;
    }
    if (row[4] != "ground")
    {
      ConnectionEndpoint second;
      if (!read_endpoint(row[4], row[5], second))
      {
        PRINT_ERROR("Invalid spring endpoint.\n");
        return false;
      }
      spring.second = second;
    }
    else if (row[5] != "0")
    {
      PRINT_ERROR("Invalid ground spring endpoint.\n");
      return false;
    }
    spring.id = number(row[0]);
    spring.name = row[1];
    spring.direction = number(row[6]);
    spring.stiffness_si = QString::fromStdString(row[7]).toDouble();
    bool valid_step = false;
    spring.history_step = QString::fromStdString(row[8]).toInt(&valid_step);
    if (!valid_step || !result.save(spring, false, error))
    {
      PRINT_ERROR("Cannot restore spring %d: %s\n", spring.id, error.c_str());
      return false;
    }
  }
  connections = std::move(result);
  return true;
}

bool CoreConnectionsPersistence::write(HDF5Tool& file, const CoreConnections& connections)
{
  Rows points;
  Rows bodies;
  for (const auto& entry : connections.points())
  {
    const auto& point = entry.second;
    std::vector<std::string> row{std::to_string(point.id), point.name, point.geometry, std::to_string(point.entity)};
    for (double coordinate : point.position)
    {
      row.push_back(QString::number(coordinate, 'g', 17).toStdString());
    }
    row.push_back(std::to_string(point.rotation_vertex));
    row.push_back(std::to_string(point.history_step));
    points.push_back(std::move(row));
  }
  for (const auto& entry : connections.bodies())
  {
    const auto& body = entry.second;
    bodies.push_back({std::to_string(body.id), body.name, body.geometry, std::to_string(body.entity), std::to_string(body.point), body.owns_point ? "1" : "0"});
  }
  Rows joints;
  for (const auto& entry : connections.joints())
  {
    const auto& joint = entry.second;
    std::vector<std::string> row{std::to_string(joint.id), joint.name, joint.type, std::to_string(joint.point_a), std::to_string(joint.point_b)};
    for (double component : joint.axis)
    {
      row.push_back(QString::number(component, 'g', 17).toStdString());
    }
    row.push_back(joint.coaxial ? "1" : "0");
    row.push_back(joint.owns_endpoints ? "1" : "0");
    row.push_back(std::to_string(joint.history_step));
    joints.push_back(std::move(row));
  }
  Rows attachments;
  for (const auto& entry : connections.attachments())
  {
    const auto& a = entry.second;
    attachments.push_back({std::to_string(a.id),a.name,a.geometry,std::to_string(a.entity),std::to_string(a.point),
      std::to_string(a.moments),a.resultant ? "1" : "0",a.mean ? "1" : "0",a.owns_point ? "1" : "0"});
  }
  Rows masses;
  for (const auto& entry : connections.point_masses())
  {
    const auto& mass = entry.second;
    masses.push_back({std::to_string(mass.id), mass.name, mass.endpoint.keyword(), std::to_string(mass.endpoint.id),
      QString::number(mass.mass_si, 'g', 17).toStdString(), std::to_string(mass.history_step)});
  }
  Rows springs;
  for (const auto& entry : connections.springs())
  {
    const auto& spring = entry.second;
    springs.push_back({std::to_string(spring.id), spring.name, spring.first.keyword(), std::to_string(spring.first.id),
      spring.second ? spring.second->keyword() : "ground", spring.second ? std::to_string(spring.second->id) : "0",
      std::to_string(spring.direction), QString::number(spring.stiffness_si, 'g', 17).toStdString(),
      std::to_string(spring.history_step)});
  }
  return file.ensureGroup(group) && file.write_dataset_string_rank_2("point_masses", group, masses)
    && file.write_dataset_string_rank_2("springs", group, springs) && file.write_dataset_string_rank_2("distributing",group,attachments) && file.write_dataset_string_rank_2("points", group, points)
    && file.write_dataset_string_rank_2("rigid_bodies", group, bodies) && file.write_dataset_string_rank_2("joints", group, joints);
}
