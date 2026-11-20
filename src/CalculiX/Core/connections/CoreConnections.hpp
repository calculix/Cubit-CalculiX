#pragma once
#include "ConnectionPointMass.hpp"
#include "ConnectionSpring.hpp"
#include <array>
#include <map>
#include <string>

struct ConnectionPoint
{
  int id = 0;
  std::string name;
  std::string geometry;
  int entity = 0;
  std::array<double, 3> position{};
  int rotation_vertex = 0;
  int history_step = -1; // -1 selects all mechanical steps, zero disables output.
};

struct ConnectionRigidBody
{
  int id = 0;
  std::string name;
  std::string geometry;
  int entity = 0;
  int point = 0;
  bool owns_point = false;
};

struct DistributingAttachment
{
  int id = 0;
  std::string name;
  std::string geometry;
  int entity = 0;
  int point = 0;
  int moments = 7;
  bool resultant = true;
  bool mean = false;
  bool owns_point = false;
};

struct ConnectionJoint
{
  int id = 0;
  std::string name;
  std::string type;
  int point_a = 0;
  int point_b = 0; // Zero selects ground.
  std::array<double, 3> axis{0, 0, 1};
  bool coaxial = false;
  bool owns_endpoints = false;
  int history_step = -1;
};

class CoreConnections
{
public:
  bool save(const ConnectionPointMass& mass, bool modify, std::string& error);
  bool save(const ConnectionSpring& spring, bool modify, std::string& error);
  bool remove_point_mass(int id);
  bool remove_spring(int id);
  const std::map<int, ConnectionPointMass>& point_masses() const;
  const std::map<int, ConnectionSpring>& springs() const;
  std::string discrete_user(int point) const;
  void clear_history_step(int step);
  bool add(const DistributingAttachment& attachment);
  bool remove_attachment(int id);
  const std::map<int, DistributingAttachment>& attachments() const;
  bool add(const ConnectionPoint& point);
  bool add(const ConnectionRigidBody& body);
  bool add(const ConnectionJoint& joint);
  bool remove_joint(int id);
  const std::map<int, ConnectionJoint>& joints() const;
  bool set_joint_history_step(int joint, int step);
  bool set_history_step(int point, int step);
  bool remove_point(int id);
  bool remove_body(int id);
  const std::map<int, ConnectionPoint>& points() const;
  const std::map<int, ConnectionRigidBody>& bodies() const;
private:
  bool validate_endpoint(const ConnectionEndpoint& endpoint, std::string& error) const;
  std::map<int, ConnectionPointMass> point_masses_;
  std::map<int, ConnectionSpring> springs_;
  std::map<int, DistributingAttachment> attachments_;
  std::map<int, ConnectionJoint> joints_;
  std::map<int, ConnectionPoint> points_;
  std::map<int, ConnectionRigidBody> bodies_;
};
