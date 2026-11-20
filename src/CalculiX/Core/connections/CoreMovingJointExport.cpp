#include "CoreMovingJointExport.hpp"
#include "CoreConnections.hpp"
#include "Core/export/CoreExportNodeAllocator.hpp"
#include <algorithm>
#include <cmath>
#include <ostream>

namespace
{
using Position = CoreMovingJointExport::Position;

Position cross(const Position& a, const Position& b)
{
  return {a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]};
}

QJsonArray coordinates(const Position& p)
{
  return {p[0],p[1],p[2]};
}

bool node(const Position& p, int& id, CoreExportNodeAllocator& allocator, std::ostream& nodes, std::string& error)
{
  if (!allocator.next(id))
  {
    error = "No node ID available for moving joint.";
    return false;
  }
  nodes << "*NODE\n" << id << ", " << p[0] << ", " << p[1] << ", " << p[2] << "\n";
  return true;
}
}

bool CoreMovingJointExport::add_markers(bool guide_side, const Position& origin, const Position& axis,
  double span, CoreExportNodeAllocator& allocator, std::ostream& nodes, std::ostream& members, std::string& error)
{
  const double norm = std::hypot(axis[0],axis[1],axis[2]);
  Position n{axis[0]/norm,axis[1]/norm,axis[2]/norm};
  auto& point = guide_side ? guide_position : tip_position;
  auto& id = guide_side ? guide : tip;
  for (int i = 0; i < 3; ++i)
  {
    point[i] = origin[i] + span*n[i];
  }
  if (!node(point,id,allocator,nodes,error))
  {
    return false;
  }
  members << id << "\n";
  if (!guide_side)
  {
    helpers.append(QJsonArray{id,coordinates(point)});
    return true;
  }
  origin_b = origin;
  Position base{};
  const auto component = std::min_element(n.begin(),n.end(),[](double a,double b)
  {
    return std::abs(a) < std::abs(b);
  });
  base[component-n.begin()] = 1;
  const auto u = cross(n,base);
  const auto w = cross(n,u);
  const double scale = span / (std::sqrt(2.)*std::hypot(u[0],u[1],u[2]));
  for (int plane = 0; plane < 2; ++plane)
  {
    Position p;
    for (int i = 0; i < 3; ++i)
    {
      p[i] = origin[i] + scale*(u[i] + (plane ? w[i] : -w[i]));
    }
    if (!node(p,planes[plane],allocator,nodes,error))
    {
      return false;
    }
    helpers.append(QJsonArray{planes[plane],coordinates(p)});
    members << planes[plane] << "\n";
  }
  return true;
}

bool CoreMovingJointExport::append(const ConnectionJoint& joint, int a, int b, CoreExportNodeAllocator& allocator,
  std::ostream& nodes, std::ostream& elements, std::ostream& constraints, std::set<std::pair<int,int>>& dependent,
  QJsonObject& binding, std::string& error)
{
  QJsonArray springs;
  const bool hinge = joint.type == "revolute";
  if (hinge)
  {
    for (int dof = 1; dof <= 3; ++dof)
    {
      if (!dependent.emplace(a,dof).second)
      {
        error = "Repeated dependent displacement on joint " + joint.name;
        return false;
      }
      constraints << "*EQUATION\n2\n" << a << ", " << dof << ", 1., " << b << ", " << dof << ", -1.\n";
    }
  }
  for (int station = 0; station < (hinge ? 1 : 2); ++station)
  {
    const auto& p = station ? origin_b : tip_position;
    const int physical = station ? a : tip;
    for (int plane : planes)
    {
      int follower = 0;
      int multiplier = 0;
      if (!node(p,follower,allocator,nodes,error) || !node(p,multiplier,allocator,nodes,error))
      {
        return false;
      }
      helpers.append(QJsonArray{follower,coordinates(p)});
      helpers.append(QJsonArray{multiplier,coordinates(p)});
      constraints << "*MPC\nPLANE, " << b << ", " << guide << ", " << plane << ", " << follower << "\n";
      // At the free multiplier, equal and opposite coefficients impose
      // equality of marker and follower displacement in each component.
      for (int dof = 1; dof <= 3; ++dof)
      {
        for (int endpoint : {physical,follower})
        {
          int element = 0;
          if (!allocator.next_element(element))
          {
            error = "No element ID available for joint " + joint.name;
            return false;
          }
          const auto set = "CCX_JOINT_" + std::to_string(joint.id) + "_AUX_" + std::to_string(element);
          if (!allocator.reserve_set(set))
          {
            error = "Reserved joint element set already exists: " + set;
            return false;
          }
          elements << "*ELEMENT,TYPE=SPRING2,ELSET=" << set << "\n" << element << "," << endpoint << "," << multiplier
            << "\n*SPRING,ELSET=" << set << "\n" << dof << "," << dof << "\n" << (endpoint == physical ? "100." : "-100.") << "\n";
          springs.append(QJsonArray{element,endpoint,multiplier});
        }
      }
    }
  }
  binding.insert("guide",guide);
  binding.insert("guide_position",coordinates(guide_position));
  binding.insert("helper_nodes",helpers);
  binding.insert("helper_elements",springs);
  return true;
}
