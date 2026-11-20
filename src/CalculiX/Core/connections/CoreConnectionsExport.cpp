#include "CoreConnectionsExport.hpp"
#include "CoreConnections.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "CoreDiscreteElementExport.hpp"
#include "CoreDistributingExport.hpp"
#include "CoreJointGeometry.hpp"
#include "CoreJointExport.hpp"
#include "CoreMovingJointExport.hpp"
#include "Core/export/CoreExportNodeAllocator.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitInterface.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <limits>
#include <locale>
#include <set>
#include <sstream>

namespace
{
QJsonArray history_steps(const CoreSteps& steps, int assignment)
{
  QJsonArray history;
  int ordinal = 0;
  for (const auto& step : steps.steps_data)
  {
    ++ordinal;
    if (step.size() >= 4 && (assignment == -1 || assignment == step[0]) &&
      (step[3] == 2 || step[3] == 6 || step[3] == 7 || step[3] == 8))
    {
      history.append(ordinal);
    }
  }
  return history;
}

bool spans_plane(const std::set<int>& members)
{
  if (members.size() < 3)
  {
    return false;
  }
  const auto origin = CubitInterface::get_nodal_coordinates(*members.begin());
  std::array<double, 3> axis{};
  double longest = 0;
  for (int id : members)
  {
    auto v = CubitInterface::get_nodal_coordinates(id);
    for (int i = 0; i < 3; ++i)
    {
      v[i] -= origin[i];
    }
    const double length = std::hypot(v[0], v[1], v[2]);
    if (length > longest)
    {
      longest = length;
      axis = v;
    }
  }
  for (int id : members)
  {
    auto v = CubitInterface::get_nodal_coordinates(id);
    for (int i = 0; i < 3; ++i)
    {
      v[i] -= origin[i];
    }
    const double area = std::hypot(axis[1]*v[2]-axis[2]*v[1], axis[2]*v[0]-axis[0]*v[2], axis[0]*v[1]-axis[1]*v[0]);
    if (area > longest * longest * 1e-12)
    {
      return true;
    }
  }
  return false;
}
}

bool CoreConnectionsExport::prepare(const CoreConnections& connections, CoreSteps& steps, const std::vector<int>& mesh_nodes, CoreExportNodeAllocator& allocator,
  std::string& nodes, std::string& elements, std::string& constraints, std::string& error)
{
  nodes.clear();
  elements.clear();
  constraints.clear();
  if (connections.bodies().empty() && connections.attachments().empty() && connections.point_masses().empty() && connections.springs().empty())
  {
    return true;
  }
  std::set<int> structural(mesh_nodes.begin(), mesh_nodes.end());
  std::set<int> assigned;
  std::set<int> control_nodes;
  std::set<int> rotation_nodes;

  std::ostringstream generated;
  generated.imbue(std::locale::classic());
  generated << std::setprecision(17);
  std::ostringstream cards;
  if (!connections.point_masses().empty() || !connections.springs().empty())
  {
    generated << std::scientific << std::setprecision(12);
  }
  cards.imbue(std::locale::classic());
  cards << std::setprecision(17);
  std::map<int, std::array<double, 3>> positions;
  std::map<int, std::array<double, 3>> axes;
  std::map<int, std::array<int, 2>> roles;
  bool finite = false;
  bool nonlinear_static = true;
  bool nonlinear_step = false;
  std::map<int,CoreMovingJointExport> moving;
  std::ostringstream joint_elements;
  joint_elements.imbue(std::locale::classic());
  std::map<int,int> guide_nodes;
  std::map<int,std::array<double,3>> guide_positions;
  if (!connections.joints().empty())
  {
    for (const auto& step : steps.steps_data)
    {
      if (step.size() < 4)
      {
        error = "Incomplete step definition while preparing joints.";
        return false;
      }
      const int index = steps.get_parameter_data_id_from_parameter_id(step[2]);
      if (index < 0 || steps.parameter_data.at(index).size() < 3)
      {
        error = "Missing step parameters while preparing joints.";
        return false;
      }
      const auto& parameters = steps.parameter_data.at(index);
      if (!parameters[2].empty())
      {
        nonlinear_step = parameters[2] == "YES";
      }
      finite = finite || nonlinear_step;
      nonlinear_static = nonlinear_static && nonlinear_step && step[3] == 2 && parameters[1].empty();
    }
  }
  for (const auto& entry : connections.joints())
  {
    const auto& joint = entry.second;
    if (finite && joint.point_b && (joint.type == "revolute" || joint.type == "cylindrical"))
    {
      if (!nonlinear_static)
      {
        error = "Moving " + joint.type + " joint " + joint.name + " requires nonlinear static steps throughout the analysis.";
        return false;
      }
      generated << std::scientific << std::setprecision(12);
      moving.try_emplace(joint.id);
    }
    axes[joint.id] = joint.axis;
    if (joint.coaxial)
    {
      std::array<double, 3> center;
      if (!CoreJointGeometry::coaxial(connections.points().at(joint.point_a).entity, connections.points().at(joint.point_b).entity, center, axes[joint.id], error))
      {
        return false;
      }
      positions[joint.point_a] = center;
      positions[joint.point_b] = center;
    }
  }
  SimUnits::QuantityUnit length;
  if (!framework_units().quantity("length", length, error))
  {
    return false;
  }
  for (const auto& entry : connections.bodies())
  {
    const auto& body = entry.second;
    const auto& point = connections.points().at(body.point);
    const auto selection = CubitInterface::parse_cubit_list("node", "in " + body.geometry + " " + std::to_string(body.entity));
    const std::set<int> members(selection.begin(), selection.end());
    for (int id : members)
    {
      if (!structural.count(id) || rotation_nodes.count(id) || !assigned.insert(id).second)
      {
        error = "Missing or multiply attached node " + std::to_string(id) + " on rigid body " + body.name;
        return false;
      }
    }
    if (members.empty() || (point.rotation_vertex == 0 && !spans_plane(members)))
    {
      error = "Rigid body " + body.name + " requires three non-collinear attachment nodes.";
      return false;
    }
    auto position = point.position;
    if (positions.count(point.id))
    {
      position = positions.at(point.id);
    }
    else if (point.geometry != "xyz")
    {
      if (CubitInterface::parse_cubit_list(point.geometry, std::to_string(point.entity)).empty())
      {
        error = "Missing placement geometry for RP " + point.name;
        return false;
      }
      position = CubitInterface::get_center_point(point.geometry, point.entity);
    }
    positions[point.id] = position;
    for (double& coordinate : position)
    {
      double si = 0;
      if (!framework_units().geometry_to_si(coordinate, si, error) || !framework_units().to_solver(si, length.dimension, "calculix", coordinate, error))
      {
        return false;
      }
    }

    int ref = 0;
    int rot = 0;
    const std::string prefix = "CCX_RP_" + std::to_string(point.id);
    if (!allocator.reserve_set(prefix + "_REF") || !allocator.reserve_set(prefix + "_ROT") ||
        !allocator.reserve_set("CCX_RB_" + std::to_string(body.id) + "_MEMBERS"))
    {
      error = "A connection set name is already in use for rigid body " + body.name;
      return false;
    }
    if (point.geometry == "vertex")
    {
      ref = CubitInterface::get_vertex_node(point.entity);
    }
    const bool generated_ref = !structural.count(ref);
    const bool generated_rot = point.rotation_vertex == 0;
    if (!generated_rot)
    {
      rot = CubitInterface::get_vertex_node(point.rotation_vertex);
      if (generated_ref || !structural.count(rot) || assigned.count(rot))
      {
        error = "Explicit control vertices must have distinct exported nodes for RP " + point.name;
        return false;
      }
    }
    if ((generated_ref && !allocator.next(ref)) || (generated_rot && !allocator.next(rot)))
    {
      error = "No control node IDs available for RP " + point.name;
      return false;
    }
    if (!control_nodes.insert(ref).second || !control_nodes.insert(rot).second)
    {
      error = "A control node belongs to multiple roles or reference points: " + point.name;
      return false;
    }
    rotation_nodes.insert(rot);
    auto write_control = [&](const char* role, int id, bool create)
    {
      if (create)
      {
        generated << "*NODE, NSET=" << prefix << role << "\n" << id << ", " << position[0] << ", " << position[1] << ", " << position[2] << "\n";
      }
      else
      {
        generated << "*NSET, NSET=" << prefix << role << "\n" << id << "\n";
      }
    };
    write_control("_REF", ref, generated_ref);
    write_control("_ROT", rot, generated_rot);
    QJsonObject binding{{"id", point.id}, {"name", QString::fromStdString(point.name)},
      {"ref", ref}, {"rot", rot}, {"history", history_steps(steps, point.history_step)}, {"position", QJsonArray{position[0],position[1],position[2]}},
      {"generated_ref", generated_ref}, {"generated_rot", generated_rot}, {"body", body.id}, {"body_name", QString::fromStdString(body.name)}};
    generated << "** CUBIT_SIM_RP " << QJsonDocument(binding).toJson(QJsonDocument::Compact).toBase64().toStdString() << "\n";
    roles[point.id] = {ref, rot};
    const std::string set = "CCX_RB_" + std::to_string(body.id) + "_MEMBERS";
    cards << "*NSET, NSET=" << set << "\n";
    for (int id : members)
    {
      cards << id << "\n";
    }
    for (const auto& item : connections.joints())
    {
      const auto& joint = item.second;
      const bool bearing = moving.count(joint.id) && (joint.point_a == point.id || joint.point_b == point.id);
      if (!bearing && (!finite || joint.type != "prismatic" || joint.point_b != point.id))
      {
        continue;
      }
      const auto& axis = axes.at(joint.id);
      const double norm = std::hypot(axis[0], axis[1], axis[2]);
      // Scale the guide to its mesh so it remains distinct at model precision.
      double radius = 0;
      const auto& origin = positions.at(point.id);
      for (int id : members)
      {
        const auto xyz = CubitInterface::get_nodal_coordinates(id);
        radius = std::max(radius, std::hypot(xyz[0]-origin[0], xyz[1]-origin[1], xyz[2]-origin[2]));
      }
      double si = 0;
      double span = 0;
      if (radius == 0 || !framework_units().geometry_to_si(radius, si, error) ||
          !framework_units().to_solver(si, length.dimension, "calculix", span, error))
      {
        error = "Cannot determine a guide length for joint " + joint.name;
        return false;
      }
      if (bearing)
      {
        if (!moving.at(joint.id).add_markers(point.id == joint.point_b,position,axis,span,allocator,generated,cards,error))
        {
          return false;
        }
        continue;
      }
      int marker = 0;
      if (!allocator.next(marker))
      {
        error = "No guide node ID available for joint " + joint.name;
        return false;
      }
      auto xyz = position;
      for (int i = 0; i < 3; ++i)
      {
        xyz[i] += span * (axis[i] / norm);
      }
      generated << "*NODE\n" << marker << ", " << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << "\n";
      cards << marker << "\n";
      guide_nodes.emplace(joint.id, marker);
      guide_positions.emplace(joint.id, xyz);
    }
    cards << "*RIGID BODY, NSET=" << set << ", REF NODE=" << ref << ", ROT NODE=" << rot << "\n";
  }
  std::set<std::pair<int,int>> dependent;
  for (const auto& entry : connections.joints())
  {
    const auto& joint = entry.second;
    std::array<int, 2> b{};
    if (joint.point_b > 0)
    {
      b = roles.at(joint.point_b);
      const auto& p = positions.at(joint.point_a);
      const auto& q = positions.at(joint.point_b);
      if (std::hypot(p[0]-q[0], p[1]-q[1], p[2]-q[2]) > 1e-9)
      {
        error = "Joint endpoints must coincide: " + joint.name;
        return false;
      }
    }
    const auto& axis = axes.at(joint.id);
    QJsonObject binding{{"id",joint.id},{"name",QString::fromStdString(joint.name)},{"type",QString::fromStdString(joint.type)},
      {"a",joint.point_a},{"b",joint.point_b},{"axis",QJsonArray{axis[0],axis[1],axis[2]}}};
    binding.insert("history", history_steps(steps, joint.history_step));
    const int guide = guide_nodes.count(joint.id) ? guide_nodes.at(joint.id) : 0;
    if (guide)
    {
      const auto& xyz = guide_positions.at(joint.id);
      binding.insert("guide", guide);
      binding.insert("guide_position", QJsonArray{xyz[0],xyz[1],xyz[2]});
    }
    if (moving.count(joint.id))
    {
      if (!moving.at(joint.id).append(joint,roles.at(joint.point_a)[0],b[0],allocator,generated,joint_elements,cards,dependent,binding,error))
      {
        return false;
      }
    }
    else if (!CoreJointExport::append(joint, roles.at(joint.point_a), b, axes.at(joint.id), guide, dependent, cards, error))
    {
      return false;
    }
    generated << "** CUBIT_SIM_JOINT " << QJsonDocument(binding).toJson(QJsonDocument::Compact).toBase64().toStdString() << "\n";
  }
  if (!CoreDistributingExport::append(connections, allocator, assigned, generated, cards, error))
  {
    return false;
  }
  std::ostringstream discrete;
  discrete.imbue(std::locale::classic());
  if (!CoreDiscreteElementExport::append(connections, steps, allocator, structural, roles, positions, generated, discrete, error))
  {
    return false;
  }
  elements = joint_elements.str() + discrete.str();
  nodes = generated.str();
  constraints = cards.str();
  return true;
}
