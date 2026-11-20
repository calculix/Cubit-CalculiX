#include "CoreConnections.hpp"
#include <algorithm>
#include <cmath>

bool CoreConnections::add(const ConnectionPoint& point)
{
  if (point.id <= 0 || point.name.empty() || point.history_step < -1 || point.rotation_vertex < 0 ||
      (point.rotation_vertex > 0 && (point.geometry != "vertex" || point.rotation_vertex == point.entity)))
  {
    return false;
  }
  if (point.geometry == "xyz")
  {
    if (point.entity != 0 || !std::all_of(point.position.begin(), point.position.end(), [](double value) { return std::isfinite(value); }))
    {
      return false;
    }
  }
  else if (point.entity <= 0 || (point.geometry != "vertex" && point.geometry != "curve" && point.geometry != "surface" && point.geometry != "volume"))
  {
    return false;
  }
  return points_.emplace(point.id, point).second;
}

bool CoreConnections::add(const ConnectionRigidBody& body)
{
  if (body.id <= 0 || body.name.empty() || body.entity <= 0 || points_.count(body.point) == 0)
  {
    return false;
  }
  if (body.geometry != "curve" && body.geometry != "surface" && body.geometry != "volume" && body.geometry != "block" && body.geometry != "nodeset")
  {
    return false;
  }
  for (const auto& existing : bodies_)
  {
    if (existing.second.point == body.point)
    {
      return false;
    }
  }
  for (const auto& entry : attachments_)
  {
    if (entry.second.point == body.point)
    {
      return false;
    }
  }
  return bodies_.emplace(body.id, body).second;
}

bool CoreConnections::set_history_step(int point, int step)
{
  auto found = points_.find(point);
  if (found == points_.end() || step < -1)
  {
    return false;
  }
  found->second.history_step = step;
  return true;
}

bool CoreConnections::remove_point(int id)
{
  if (!discrete_user(id).empty())
  {
    return false;
  }
  for (const auto& entry : bodies_)
  {
    if (entry.second.point == id)
    {
      return false;
    }
  }
  for (const auto& entry : attachments_)
  {
    if (entry.second.point == id)
    {
      return false;
    }
  }
  return points_.erase(id) != 0;
}

bool CoreConnections::remove_body(int id)
{
  auto body = bodies_.find(id);
  if (body == bodies_.end() || !discrete_user(body->second.point).empty())
  {
    return false;
  }
  for (const auto& entry : joints_)
  {
    if (entry.second.point_a == body->second.point || entry.second.point_b == body->second.point)
    {
      return false;
    }
  }
  const auto point = body->second.point;
  const bool owns_point = body->second.owns_point;
  bodies_.erase(body);
  if (owns_point)
  {
    remove_point(point);
  }
  return true;
}

bool CoreConnections::add(const ConnectionJoint& joint)
{
  if (joint.id <= 0 || joint.history_step < -1 || joint.name.empty() || joint.point_a == joint.point_b ||
      (joint.type != "fixed" && joint.type != "spherical" && joint.type != "revolute" &&
       joint.type != "prismatic" && joint.type != "cylindrical"))
  {
    return false;
  }
  const double norm = std::hypot(joint.axis[0], joint.axis[1], joint.axis[2]);
  if (!std::isfinite(norm) || norm == 0)
  {
    return false;
  }
  const auto attached = [&](int point)
  {
    return std::any_of(bodies_.begin(), bodies_.end(), [point](const auto& entry) { return entry.second.point == point; });
  };
  if (!attached(joint.point_a) || (joint.point_b != 0 && !attached(joint.point_b)))
  {
    return false;
  }
  if (joint.owns_endpoints != joint.coaxial)
  {
    return false;
  }
  if (joint.coaxial && (joint.point_b == 0 || points_.at(joint.point_a).geometry != "surface" ||
      points_.at(joint.point_b).geometry != "surface" || points_.at(joint.point_a).entity == points_.at(joint.point_b).entity))
  {
    return false;
  }
  return joints_.emplace(joint.id, joint).second;
}

bool CoreConnections::set_joint_history_step(int joint, int step)
{
  auto found = joints_.find(joint);
  if (found == joints_.end() || step < -1)
  {
    return false;
  }
  found->second.history_step = step;
  return true;
}

bool CoreConnections::remove_joint(int id)
{
  auto found = joints_.find(id);
  if (found == joints_.end())
  {
    return false;
  }
  const auto joint = found->second;
  joints_.erase(found);
  if (joint.owns_endpoints)
  {
    for (int point : {joint.point_a, joint.point_b})
    {
      auto body = std::find_if(bodies_.begin(), bodies_.end(), [point](const auto& entry) { return entry.second.point == point; });
      if (body != bodies_.end() && remove_body(body->first))
      {
        remove_point(point);
      }
    }
  }
  return true;
}

const std::map<int, ConnectionJoint>& CoreConnections::joints() const
{
  return joints_;
}

const std::map<int, ConnectionPoint>& CoreConnections::points() const
{
  return points_;
}

const std::map<int, ConnectionRigidBody>& CoreConnections::bodies() const
{
  return bodies_;
}

bool CoreConnections::add(const DistributingAttachment& attachment)
{
  if (attachment.id <= 0 || attachment.entity <= 0 || attachment.name.empty() || !points_.count(attachment.point) || !discrete_user(attachment.point).empty() ||
      (attachment.geometry != "surface" && attachment.geometry != "sideset") || attachment.moments < 0 || attachment.moments > 7)
  {
    return false;
  }
  for (const auto& entry : bodies_)
  {
    if (entry.second.point == attachment.point)
    {
      return false;
    }
  }
  for (const auto& entry : attachments_)
  {
    if (entry.second.point == attachment.point)
    {
      return false;
    }
  }
  return attachments_.emplace(attachment.id, attachment).second;
}

bool CoreConnections::remove_attachment(int id)
{
  const auto found = attachments_.find(id);
  if (found == attachments_.end())
  {
    return false;
  }
  const auto attachment = found->second;
  attachments_.erase(found);
  if (attachment.owns_point)
  {
    remove_point(attachment.point);
  }
  return true;
}

const std::map<int, DistributingAttachment>& CoreConnections::attachments() const
{
  return attachments_;
}

bool CoreConnections::validate_endpoint(const ConnectionEndpoint& endpoint, std::string& error) const
{
  if (endpoint.kind == ConnectionEndpointKind::Vertex && endpoint.id > 0)
  {
    return true;
  }
  if (endpoint.kind != ConnectionEndpointKind::ReferencePoint || !points_.count(endpoint.id))
  {
    error = "Select an existing vertex or reference point.";
    return false;
  }
  for (const auto& entry : attachments_)
  {
    if (entry.second.point == endpoint.id)
    {
      error = "RP " + std::to_string(endpoint.id) + " distributes a load and cannot supply endpoint motion.";
      return false;
    }
  }
  return true;
}

bool CoreConnections::save(const ConnectionPointMass& mass, bool modify, std::string& error)
{
  if (!mass.validate(error) || !validate_endpoint(mass.endpoint, error))
  {
    return false;
  }
  if (modify != (point_masses_.count(mass.id) != 0))
  {
    error = modify ? "Unknown point mass ID." : "Point mass ID already exists.";
    return false;
  }
  point_masses_.insert_or_assign(mass.id, mass);
  return true;
}

bool CoreConnections::save(const ConnectionSpring& spring, bool modify, std::string& error)
{
  if (!spring.validate(error) || !validate_endpoint(spring.first, error) ||
      (spring.second && !validate_endpoint(*spring.second, error)))
  {
    return false;
  }
  if (modify != (springs_.count(spring.id) != 0))
  {
    error = modify ? "Unknown spring ID." : "Spring ID already exists.";
    return false;
  }
  springs_.insert_or_assign(spring.id, spring);
  return true;
}

std::string CoreConnections::discrete_user(int point) const
{
  for (const auto& entry : point_masses_)
  {
    if (entry.second.endpoint.uses_point(point))
    {
      return "point mass " + std::to_string(entry.first);
    }
  }
  for (const auto& entry : springs_)
  {
    if (entry.second.uses_point(point))
    {
      return "spring " + std::to_string(entry.first);
    }
  }
  return {};
}

bool CoreConnections::remove_point_mass(int id)
{
  return point_masses_.erase(id) != 0;
}

bool CoreConnections::remove_spring(int id)
{
  return springs_.erase(id) != 0;
}

const std::map<int, ConnectionPointMass>& CoreConnections::point_masses() const
{
  return point_masses_;
}

const std::map<int, ConnectionSpring>& CoreConnections::springs() const
{
  return springs_;
}

void CoreConnections::clear_history_step(int step)
{
  for (auto& entry : joints_)
  {
    if (entry.second.history_step == step)
    {
      entry.second.history_step = 0;
    }
  }
  for (auto& entry : points_)
  {
    if (entry.second.history_step == step)
    {
      entry.second.history_step = 0;
    }
  }
  for (auto& entry : point_masses_)
  {
    if (entry.second.history_step == step)
    {
      entry.second.history_step = 0;
    }
  }
  for (auto& entry : springs_)
  {
    if (entry.second.history_step == step)
    {
      entry.second.history_step = 0;
    }
  }
}
