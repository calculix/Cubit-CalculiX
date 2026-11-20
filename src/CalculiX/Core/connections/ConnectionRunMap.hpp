#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <array>
#include <map>
#include <string>

struct ConnectionResultBinding
{
  int id = 0;
  std::string name;
  int ref = 0;
  int rot = 0;
  std::array<double, 3> position{};
  bool generated_ref = true;
  bool generated_rot = true;
  int body = 0;
  std::string body_name;
  QJsonArray history;
};

struct JointResultBinding
{
  int id = 0;
  std::string name;
  std::string type;
  int a = 0;
  int b = 0;
  std::array<double, 3> axis{};
  QJsonArray history;
  QJsonArray helper_nodes;
  QJsonArray helper_elements;
  int guide = 0;
  std::array<double,3> guide_position{};
};

struct DistributedResultBinding
{
  int id = 0;
  int point = 0;
  int ref = 0;
  std::string name;
  std::string point_name;
  std::array<double, 3> position{};
  std::map<int,double> weights;
  QJsonArray prescribed;
};

struct DiscreteResultBinding
{
  int id = 0;
  int element = 0;
  std::string name;
  std::string type;
  int a = 0;
  int b = 0;
  int direction = 0;
  double value = 0;
  std::array<double,3> position_a{};
  std::array<double,3> position_b{};
  std::map<int,bool> history; // Solver step ordinal -> finite geometry.
};

class ConnectionRunMap
{
public:
  std::map<int, ConnectionResultBinding> points;
  std::map<int, DiscreteResultBinding> discrete; // Keyed by global solver element ID.
  std::map<int, JointResultBinding> joints;
  std::map<int, DistributedResultBinding> distributed;
  std::map<int,std::array<double,3>> joint_nodes;
  std::map<int,std::array<int,2>> joint_elements;
  bool read_marker(const std::string& line, std::string& error);
  bool restore(const QJsonObject& object, std::string& error);
  QJsonObject json() const;
  bool valid() const;
};
