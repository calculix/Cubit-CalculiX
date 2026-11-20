#pragma once
#include "CoreConnections.hpp"
#include <vector>
#include <set>

class CalculiXConnectionsInterface
{
public:
  bool save(const ConnectionPointMass& mass, bool modify, std::string& error) const;
  bool save(const ConnectionSpring& spring, bool modify, std::string& error) const;
  bool point_mass(int id, ConnectionPointMass& record) const;
  bool spring(int id, ConnectionSpring& record) const;
  std::vector<ConnectionPointMass> point_masses() const;
  std::vector<ConnectionSpring> springs() const;
  bool remove_point_mass(int id) const;
  bool remove_spring(int id) const;
  bool set_discrete_history_step(int id, bool spring, int step, std::string& error) const;
  bool get_rotation_node_ids(std::set<int>& nodes, std::string& error) const;
  std::vector<int> point_forces(int id) const;
  bool create_attachment(DistributingAttachment attachment, bool center, std::string& error) const;
  std::vector<DistributingAttachment> attachments() const;
  bool remove_attachment(int id) const;
  bool has_point(int id) const;
  bool point_position(int id, std::array<double, 3>& position, std::string& error) const;
  bool prepare_export(const std::vector<int>& mesh_nodes, const std::string& filename, std::string& nodes, std::string& elements, std::string& constraints, std::string& error) const;
  bool create(const ConnectionPoint& point) const;
  bool create(const ConnectionRigidBody& body) const;
  bool create_at_center(ConnectionRigidBody body) const;
  bool create_joint(ConnectionJoint joint, int surface_a, int surface_b, std::string& error) const;
  bool set_history_step(int id, bool joint, int step) const;
  bool remove_joint(int id) const;
  std::vector<ConnectionJoint> joints() const;
  bool remove_point(int id) const;
  bool remove_body(int id) const;
  std::vector<ConnectionPoint> points() const;
  std::vector<ConnectionRigidBody> bodies() const;
};
