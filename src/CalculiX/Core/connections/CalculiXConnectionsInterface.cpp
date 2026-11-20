#include "CalculiXConnectionsInterface.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "CubitInterface.hpp"
#include "CoreConnectionsExport.hpp"
#include <utility>
#include <limits>
#include <algorithm>
#include "Core/export/CoreCustomLines.hpp"
#include "Core/export/CoreExportNodeAllocator.hpp"
#include "CoreJointGeometry.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "CubitMessage.hpp"

namespace
{
bool can_detach(int point)
{
  const auto feature = ccx_core.connections.discrete_user(point);
  if (!feature.empty())
  {
    PRINT_ERROR("RP %d is used by %s. Retarget or delete that feature first.\n", point, feature.c_str());
    return false;
  }
  return true;
}

bool commit_removal(CoreConnections&& candidate)
{
  if (ccx_core.loadsforces)
  {
    for (const auto& force : ccx_core.loadsforces->loads_data)
    {
      if (force.size() > 7 && force[7] > 0 && ccx_core.connections.points().count(force[7]) &&
          !candidate.points().count(force[7]))
      {
        PRINT_ERROR("Reference point %d is targeted by force %d. Retarget or delete the force first.\n", force[7], force[0]);
        return false;
      }
    }
  }
  ccx_core.connections = std::move(candidate);
  return true;
}

bool exists(const std::string& kind, int id)
{
  return id > 0 && !CubitInterface::parse_cubit_list(kind, std::to_string(id)).empty();
}
}

bool CalculiXConnectionsInterface::prepare_export(const std::vector<int>& mesh_nodes, const std::string& filename, std::string& nodes, std::string& elements, std::string& constraints, std::string& error) const
{
  CoreExportNodeAllocator allocator;
  const bool discrete = !ccx_core.connections.point_masses().empty() || !ccx_core.connections.springs().empty();
  bool bearing = false;
  for (const auto& entry : ccx_core.connections.joints())
  {
    if (entry.second.point_b && (entry.second.type == "revolute" || entry.second.type == "cylindrical"))
    {
      for (const auto& parameters : ccx_core.steps->parameter_data)
      {
        if (parameters.size() > 2 && parameters[2] == "YES")
        {
          bearing = true;
          break;
        }
      }
      break;
    }
  }
  if ((discrete || !ccx_core.connections.bodies().empty() || !ccx_core.connections.attachments().empty()) &&
      !allocator.prepare(mesh_nodes, ccx_core.customlines->customlines_data, filename, discrete || bearing, error))
  {
    return false;
  }
  return CoreConnectionsExport::prepare(ccx_core.connections, *ccx_core.steps, mesh_nodes, allocator, nodes, elements, constraints, error);
}

bool CalculiXConnectionsInterface::create(const ConnectionPoint& point) const
{
  CoreConnections candidate = ccx_core.connections;
  if (!candidate.add(point) || (point.geometry != "xyz" && !exists(point.geometry, point.entity)) ||
      (point.rotation_vertex > 0 && !exists("vertex", point.rotation_vertex)))
  {
    return false;
  }
  ccx_core.connections = std::move(candidate);
  return true;
}

bool CalculiXConnectionsInterface::create(const ConnectionRigidBody& body) const
{
  CoreConnections candidate = ccx_core.connections;
  if (!candidate.add(body) || !exists(body.geometry, body.entity))
  {
    return false;
  }
  ccx_core.connections = std::move(candidate);
  return true;
}

bool CalculiXConnectionsInterface::create_at_center(ConnectionRigidBody body) const
{
  if ((body.geometry != "curve" && body.geometry != "surface" && body.geometry != "volume") || !exists(body.geometry, body.entity))
  {
    return false;
  }
  auto candidate = ccx_core.connections;
  const int last = candidate.points().empty() ? 0 : candidate.points().rbegin()->first;
  if (last == (std::numeric_limits<int>::max)())
  {
    return false;
  }
  body.point = last + 1;
  body.owns_point = true;
  if (!candidate.add(ConnectionPoint{body.point, body.name + " center", body.geometry, body.entity}) || !candidate.add(body))
  {
    return false;
  }
  ccx_core.connections = std::move(candidate);
  return true;
}

bool CalculiXConnectionsInterface::create_joint(ConnectionJoint joint, int surface_a, int surface_b, std::string& error) const
{
  auto candidate = ccx_core.connections;
  if (joint.history_step > 0 && ccx_core.steps->get_steps_data_id_from_step_id(joint.history_step) < 0)
  {
    error = "Unknown history step for joint " + joint.name;
    return false;
  }
  if (joint.coaxial)
  {
    std::array<double, 3> center;
    if (surface_a == surface_b || !exists("surface", surface_a) || !exists("surface", surface_b) ||
        !CoreJointGeometry::coaxial(surface_a, surface_b, center, joint.axis, error))
    {
      if (error.empty())
      {
        error = "Select distinct existing joint faces.";
      }
      return false;
    }
    int point = candidate.points().empty() ? 0 : candidate.points().rbegin()->first;
    int body = candidate.bodies().empty() ? 0 : candidate.bodies().rbegin()->first;
    if (std::max(point, body) > (std::numeric_limits<int>::max)() - 2)
    {
      error = "No connection IDs available.";
      return false;
    }
    joint.point_a = point + 1;
    joint.point_b = point + 2;
    joint.owns_endpoints = true;
    for (int surface : {surface_a, surface_b})
    {
      const std::string label = joint.name + (surface == surface_a ? " A" : " B");
      if (!candidate.add(ConnectionPoint{++point, label, "surface", surface}) ||
          !candidate.add(ConnectionRigidBody{++body, label, "surface", surface, point}))
      {
        error = "Cannot create joint endpoints.";
        return false;
      }
    }
  }
  if (!candidate.add(joint))
  {
    error = "Check the joint ID, type, endpoint attachments and axis.";
    return false;
  }
  ccx_core.connections = std::move(candidate);
  return true;
}

std::vector<ConnectionJoint> CalculiXConnectionsInterface::joints() const
{
  std::vector<ConnectionJoint> result;
  for (const auto& entry : ccx_core.connections.joints())
  {
    result.push_back(entry.second);
  }
  return result;
}

bool CalculiXConnectionsInterface::set_history_step(int id, bool joint, int step) const
{
  if (step > 0 && ccx_core.steps->get_steps_data_id_from_step_id(step) < 0)
  {
    return false;
  }
  auto& connections = ccx_core.connections;
  if (!joint)
  {
    return connections.set_history_step(id, step);
  }
  return connections.set_joint_history_step(id, step);
}

bool CalculiXConnectionsInterface::remove_joint(int id) const
{
  const auto& joints = ccx_core.connections.joints();
  const auto found = joints.find(id);
  if (found != joints.end() && found->second.owns_endpoints &&
      (!can_detach(found->second.point_a) || !can_detach(found->second.point_b)))
  {
    return false;
  }
  auto candidate = ccx_core.connections;
  return candidate.remove_joint(id) && commit_removal(std::move(candidate));
}

bool CalculiXConnectionsInterface::remove_point(int id) const
{
  if (!can_detach(id))
  {
    return false;
  }
  auto candidate = ccx_core.connections;
  return candidate.remove_point(id) && commit_removal(std::move(candidate));
}

bool CalculiXConnectionsInterface::remove_body(int id) const
{
  const auto& bodies = ccx_core.connections.bodies();
  const auto found = bodies.find(id);
  if (found != bodies.end() && !can_detach(found->second.point))
  {
    return false;
  }
  auto candidate = ccx_core.connections;
  return candidate.remove_body(id) && commit_removal(std::move(candidate));
}

std::vector<ConnectionPoint> CalculiXConnectionsInterface::points() const
{
  std::vector<ConnectionPoint> result;
  for (const auto& entry : ccx_core.connections.points())
  {
    result.push_back(entry.second);
  }
  return result;
}

std::vector<ConnectionRigidBody> CalculiXConnectionsInterface::bodies() const
{
  std::vector<ConnectionRigidBody> result;
  for (const auto& entry : ccx_core.connections.bodies())
  {
    result.push_back(entry.second);
  }
  return result;
}

bool CalculiXConnectionsInterface::get_rotation_node_ids(std::set<int>& nodes, std::string& error) const
{
  nodes.clear();
  for (const auto& entry : ccx_core.connections.bodies())
  {
    const auto& point = ccx_core.connections.points().at(entry.second.point);
    if (point.rotation_vertex <= 0)
    {
      continue;
    }
    const int node = CubitInterface::get_vertex_node(point.rotation_vertex);
    if (node <= 0)
    {
      error = "Mesh the rotation vertex for RP " + point.name + " before resolving mechanical DOFs.";
      return false;
    }
    nodes.insert(node);
  }
  error.clear();
  return true;
}

bool CalculiXConnectionsInterface::has_point(int id) const
{
  return ccx_core.connections.points().count(id) != 0;
}

bool CalculiXConnectionsInterface::point_position(int id, std::array<double, 3>& position, std::string& error) const
{
  const auto& points = ccx_core.connections.points();
  const auto found = points.find(id);
  if (found == points.end())
  {
    error = "Unknown reference point " + std::to_string(id);
    return false;
  }
  for (const auto& entry : ccx_core.connections.joints())
  {
    const auto& joint = entry.second;
    if (joint.coaxial && (id == joint.point_a || id == joint.point_b))
    {
      std::array<double, 3> axis;
      return CoreJointGeometry::coaxial(points.at(joint.point_a).entity, points.at(joint.point_b).entity, position, axis, error);
    }
  }
  const auto& point = found->second;
  if (point.geometry == "xyz")
  {
    position = point.position;
    return true;
  }
  if (!exists(point.geometry, point.entity))
  {
    error = "Missing geometry for reference point " + point.name;
    return false;
  }
  position = CubitInterface::get_center_point(point.geometry, point.entity);
  return true;
}

std::vector<int> CalculiXConnectionsInterface::point_forces(int id) const
{
  std::vector<int> ids;
  if (ccx_core.loadsforces)
  {
    for (const auto& load : ccx_core.loadsforces->loads_data)
    {
      if (load.size() > 7 && load[7] == id)
      {
        ids.push_back(load[0]);
      }
    }
  }
  return ids;
}

bool CalculiXConnectionsInterface::create_attachment(DistributingAttachment attachment, bool center, std::string& error) const
{
  auto candidate = ccx_core.connections;
  if (!exists(attachment.geometry, attachment.entity))
  {
    error = "Select an existing surface or face-based sideset.";
    return false;
  }
  if (attachment.geometry == "surface" && CubitInterface::get_geometry_type("surface", {attachment.entity}) != std::vector<std::string>{"plane"})
  {
    error = "Select a planar surface.";
    return false;
  }
  if (center)
  {
    const int last = candidate.points().empty() ? 0 : candidate.points().rbegin()->first;
    if (attachment.geometry != "surface" || last == (std::numeric_limits<int>::max)())
    {
      error = "Automatic RP placement requires a planar Cubit surface.";
      return false;
    }
    attachment.point = last + 1;
    attachment.owns_point = true;
    if (!candidate.add(ConnectionPoint{attachment.point, attachment.name + " center", "surface", attachment.entity}))
    {
      error = "Cannot create the reference point.";
      return false;
    }
  }
  const auto dependent = candidate.discrete_user(attachment.point);
  if (!dependent.empty())
  {
    error = "This RP supplies endpoint motion for " + dependent + ". Select another RP for load distribution.";
    return false;
  }
  if (!candidate.add(attachment))
  {
    error = "Check the attachment ID, moment selection and RP. Each RP can have one attachment.";
    return false;
  }
  ccx_core.connections = std::move(candidate);
  return true;
}

bool CalculiXConnectionsInterface::remove_attachment(int id) const
{
  auto candidate = ccx_core.connections;
  return candidate.remove_attachment(id) && commit_removal(std::move(candidate));
}

std::vector<DistributingAttachment> CalculiXConnectionsInterface::attachments() const
{
  std::vector<DistributingAttachment> result;
  for (const auto& entry : ccx_core.connections.attachments())
  {
    result.push_back(entry.second);
  }
  return result;
}

namespace
{
bool vertex_available(const ConnectionEndpoint& endpoint, std::string& error)
{
  if (endpoint.kind == ConnectionEndpointKind::Vertex && !exists("vertex", endpoint.id))
  {
    error = "Unknown endpoint vertex " + std::to_string(endpoint.id) + ".";
    return false;
  }
  return true;
}

bool history_step_available(int step, std::string& error)
{
  if (step < -1 || (step > 0 && ccx_core.steps->get_steps_data_id_from_step_id(step) < 0))
  {
    error = "Select an existing step, all mechanical steps, or history off.";
    return false;
  }
  return true;
}
}

bool CalculiXConnectionsInterface::save(const ConnectionPointMass& mass, bool modify, std::string& error) const
{
  return vertex_available(mass.endpoint, error) && history_step_available(mass.history_step, error) &&
    ccx_core.connections.save(mass, modify, error);
}

bool CalculiXConnectionsInterface::save(const ConnectionSpring& spring, bool modify, std::string& error) const
{
  return vertex_available(spring.first, error) && (!spring.second || vertex_available(*spring.second, error)) &&
    history_step_available(spring.history_step, error) && ccx_core.connections.save(spring, modify, error);
}

bool CalculiXConnectionsInterface::point_mass(int id, ConnectionPointMass& record) const
{
  const auto& records = ccx_core.connections.point_masses();
  const auto found = records.find(id);
  if (found == records.end())
  {
    return false;
  }
  record = found->second;
  return true;
}

bool CalculiXConnectionsInterface::spring(int id, ConnectionSpring& record) const
{
  const auto& records = ccx_core.connections.springs();
  const auto found = records.find(id);
  if (found == records.end())
  {
    return false;
  }
  record = found->second;
  return true;
}

std::vector<ConnectionPointMass> CalculiXConnectionsInterface::point_masses() const
{
  std::vector<ConnectionPointMass> result;
  for (const auto& entry : ccx_core.connections.point_masses())
  {
    result.push_back(entry.second);
  }
  return result;
}

std::vector<ConnectionSpring> CalculiXConnectionsInterface::springs() const
{
  std::vector<ConnectionSpring> result;
  for (const auto& entry : ccx_core.connections.springs())
  {
    result.push_back(entry.second);
  }
  return result;
}

bool CalculiXConnectionsInterface::remove_point_mass(int id) const
{
  return ccx_core.connections.remove_point_mass(id);
}

bool CalculiXConnectionsInterface::remove_spring(int id) const
{
  return ccx_core.connections.remove_spring(id);
}

bool CalculiXConnectionsInterface::set_discrete_history_step(int id, bool spring, int step, std::string& error) const
{
  if (!history_step_available(step,error))
  {
    return false;
  }
  if (spring)
  {
    const auto& springs = ccx_core.connections.springs();
    auto found = springs.find(id);
    if (found == springs.end())
    {
      error = "Unknown spring ID.";
      return false;
    }
    auto updated = found->second;
    updated.history_step = step;
    return ccx_core.connections.save(updated,true,error);
  }
  const auto& masses = ccx_core.connections.point_masses();
  auto found = masses.find(id);
  if (found == masses.end())
  {
    error = "Unknown point mass ID.";
    return false;
  }
  auto updated = found->second;
  updated.history_step = step;
  return ccx_core.connections.save(updated,true,error);
}
