#include "ConnectionRunMap.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <cmath>
#include <set>
#include <sstream>
#include <locale>

namespace
{
bool vector3(const QJsonValue& value, std::array<double, 3>& output)
{
  const auto array = value.toArray();
  if (array.size() != 3)
  {
    return false;
  }
  for (int i = 0; i < 3; ++i)
  {
    output[i] = array[i].toDouble();
    if (!array[i].isDouble() || !std::isfinite(output[i]))
    {
      return false;
    }
  }
  return true;
}

bool read_discrete(const QJsonObject& json, ConnectionRunMap& map)
{
  DiscreteResultBinding item;
  item.id = json.value("id").toInt();
  item.element = json.value("element").toInt();
  item.a = json.value("a").toInt();
  item.b = json.value("b").toInt();
  item.direction = json.value("direction").toInt();
  item.name = json.value("name").toString().toStdString();
  item.type = json.value("type").toString().toStdString();
  item.value = json.value("value").toDouble();
  if (item.id <= 0 || item.element <= 0 || item.name.empty() || item.a <= 0 ||
      !(item.value > 0) || !std::isfinite(item.value) || !vector3(json.value("position_a"),item.position_a))
  {
    return false;
  }
  if (item.type == "SPRINGA")
  {
    if (item.b <= 0 || item.a == item.b || item.direction != 0 ||
        !vector3(json.value("position_b"),item.position_b) || item.position_a == item.position_b)
    {
      return false;
    }
  }
  else if (item.b != 0 || !((item.type == "MASS" && item.direction == 0) ||
                            (item.type == "SPRING1" && item.direction >= 1 && item.direction <= 3)))
  {
    return false;
  }
  for (const auto pair : json.value("history").toArray())
  {
    const auto values = pair.toArray();
    if (values.size() != 2 || values[0].toInt() <= 0 || !values[1].isBool() ||
        !item.history.emplace(values[0].toInt(), values[1].toBool()).second)
    {
      return false;
    }
  }
  return map.discrete.emplace(item.element,item).second;
}

bool attachment(const QJsonObject& json, ConnectionRunMap& map)
{
  DistributedResultBinding a;
  a.id = json.value("id").toInt();
  a.point = json.value("point").toInt();
  a.ref = json.value("ref").toInt();
  a.name = json.value("name").toString().toStdString();
  a.prescribed = json.value("prescribed").toArray();
  a.point_name = json.value("rp_name").toString().toStdString();
  if (a.id <= 0 || a.point <= 0 || a.ref <= 0 || a.name.empty() || a.point_name.empty() || !vector3(json.value("position"), a.position))
  {
    return false;
  }
  for (const auto value : json.value("weights").toArray())
  {
    const auto pair = value.toArray();
    if (pair.size() != 2 || pair[0].toInt() <= 0 || !pair[1].isDouble() || !(pair[1].toDouble() > 0) ||
        !a.weights.emplace(pair[0].toInt(),pair[1].toDouble()).second)
    {
      return false;
    }
  }
  return map.distributed.emplace(a.id,std::move(a)).second;
}

bool point(const QJsonObject& json, ConnectionRunMap& map)
{
  ConnectionResultBinding p{json.value("id").toInt(),json.value("name").toString().toStdString(),
    json.value("ref").toInt(),json.value("rot").toInt()};
  p.generated_ref = json.value("generated_ref").toBool();
  p.generated_rot = json.value("generated_rot").toBool();
  p.history = json.value("history").toArray();
  p.body = json.value("body").toInt();
  p.body_name = json.value("body_name").toString().toStdString();
  return p.id > 0 && !p.name.empty() && p.ref > 0 && p.rot > 0 && p.ref != p.rot &&
    p.body > 0 && !p.body_name.empty() && json.value("generated_ref").isBool() && json.value("generated_rot").isBool() && vector3(json.value("position"),p.position) && map.points.emplace(p.id,p).second;
}

bool joint(const QJsonObject& json, ConnectionRunMap& map)
{
  JointResultBinding j{json.value("id").toInt(),json.value("name").toString().toStdString(),
    json.value("type").toString().toStdString(),json.value("a").toInt(),json.value("b").toInt()};
  if (!json.value("history").isArray())
  {
    return false;
  }
  j.history = json.value("history").toArray();
  j.helper_nodes = json.value("helper_nodes").toArray();
  j.helper_elements = json.value("helper_elements").toArray();
  for (const auto& item : j.helper_nodes)
  {
    const auto row = item.toArray();
    std::array<double,3> position;
    if (row.size() != 2 || row[0].toInt() <= 0 || !vector3(row[1],position) ||
        !map.joint_nodes.emplace(row[0].toInt(),position).second)
    {
      return false;
    }
  }
  for (const auto& item : j.helper_elements)
  {
    const auto row = item.toArray();
    if (row.size() != 3 || row[0].toInt() <= 0 || row[1].toInt() <= 0 || row[2].toInt() <= 0 ||
        !map.joint_elements.emplace(row[0].toInt(),std::array<int,2>{row[1].toInt(),row[2].toInt()}).second)
    {
      return false;
    }
  }
  j.guide = json.value("guide").toInt();
  if (!j.helper_elements.isEmpty() || !j.helper_nodes.isEmpty())
  {
    const bool hinge = j.type == "revolute";
    if ((!hinge && j.type != "cylindrical") || !j.guide || !j.b ||
        j.helper_elements.size() != (hinge ? 12 : 24) || j.helper_nodes.size() != (hinge ? 7 : 11))
    {
      return false;
    }
  }
  if (j.guide && (j.guide < 0 || !vector3(json.value("guide_position"), j.guide_position)))
  {
    return false;
  }
  if (j.guide && !map.joint_nodes.emplace(j.guide,j.guide_position).second)
  {
    return false;
  }
  return j.id > 0 && !j.name.empty() && j.a > 0 && j.b >= 0 && j.a != j.b &&
    vector3(json.value("axis"),j.axis) && std::hypot(j.axis[0],j.axis[1],j.axis[2]) > 0 && map.joints.emplace(j.id,j).second;
}
}

bool ConnectionRunMap::read_marker(const std::string& line, std::string& error)
{
  const std::string definition = "** CUBIT_SIM_DISCRETE ";
  if (line.compare(0,definition.size(),definition) == 0)
  {
    const auto encoded = QByteArray::fromStdString(line.substr(definition.size()));
    if (!read_discrete(QJsonDocument::fromJson(QByteArray::fromBase64(encoded)).object(), *this))
    {
      error = "Invalid point mass or spring mapping.";
      return false;
    }
    return true;
  }
  const std::string history = "** CUBIT_SIM_DISCRETE_HISTORY ";
  if (line.compare(0,history.size(),history) == 0)
  {
    std::istringstream input(line.substr(history.size()));
    std::string type;
    int id, step, finite;
    if (input >> type >> id >> step >> finite && step > 0 && (finite == 0 || finite == 1))
    {
      for (auto& entry : discrete)
      {
        if (entry.second.type == type && entry.second.id == id && entry.second.history.emplace(step,finite == 1).second)
        {
          return true;
        }
      }
    }
    error = "Invalid point mass or spring history assignment.";
    return false;
  }
  const std::string da = "** CUBIT_SIM_DA ";
  if (line.compare(0, da.size(), da) == 0)
  {
    const auto json = QJsonDocument::fromJson(QByteArray::fromBase64(QByteArray::fromStdString(line.substr(da.size()))));
    if (!attachment(json.object(), *this))
    {
      error = "Invalid distributing attachment mapping.";
      return false;
    }
    return true;
  }
  const std::string load = "** CUBIT_SIM_DA_LOAD ";
  if (line.compare(0, load.size(), load) == 0)
  {
    const auto json = QJsonDocument::fromJson(QByteArray::fromBase64(QByteArray::fromStdString(line.substr(load.size())))).object();
    const auto found = distributed.find(json.value("attachment").toInt());
    if (found == distributed.end() || json.value("step").toInt() <= 0 || json.value("components").toArray().size() != 6)
    {
      error = "Invalid prescribed distributing load record.";
      return false;
    }
    found->second.prescribed.append(json);
    return true;
  }
  const std::string weight = "** CUBIT_SIM_DA_WEIGHT ";
  if (line.compare(0, weight.size(), weight) == 0)
  {
    std::istringstream input(line.substr(weight.size()));
    input.imbue(std::locale::classic());
    int id, node;
    double value;
    if (!(input >> id >> node >> value) || !distributed.count(id) || node <= 0 || !std::isfinite(value) || value <= 0 ||
        !distributed.at(id).weights.emplace(node,value).second)
    {
      error = "Invalid distributing attachment area weight.";
      return false;
    }
    return true;
  }
  const std::string rp = "** CUBIT_SIM_RP ";
  const std::string jt = "** CUBIT_SIM_JOINT ";
  const bool is_point = line.compare(0,rp.size(),rp) == 0;
  if (!is_point && line.compare(0,jt.size(),jt) != 0)
  {
    return true;
  }
  const auto encoded = QByteArray::fromStdString(line.substr(is_point ? rp.size() : jt.size()));
  const auto json = QJsonDocument::fromJson(QByteArray::fromBase64(encoded)).object();
  if (!(is_point ? point(json,*this) : joint(json,*this)))
  {
    error = "Invalid or duplicate connection mapping in the input deck.";
    return false;
  }
  return true;
}

bool ConnectionRunMap::valid() const
{
  std::set<int> nodes;
  for (const auto& entry : points)
  {
    if (!nodes.insert(entry.second.ref).second || !nodes.insert(entry.second.rot).second)
    {
      return false;
    }
  }
  std::set<std::pair<bool,int>> features;
  for (const auto& entry : discrete)
  {
    if (!features.emplace(entry.second.type == "MASS",entry.second.id).second)
    {
      return false;
    }
  }
  for (const auto& item : joint_nodes)
  {
    if (!nodes.insert(item.first).second)
    {
      return false;
    }
  }
  for (const auto& item : joint_elements)
  {
    if (discrete.count(item.first) || !nodes.count(item.second[0]) || !joint_nodes.count(item.second[1]))
    {
      return false;
    }
  }
  std::set<int> distributed_points;
  for (const auto& entry : distributed)
  {
    const auto& a = entry.second;
    if (points.count(a.point) || !distributed_points.insert(a.point).second || !nodes.insert(a.ref).second)
    {
      return false;
    }
  }
  for (const auto& entry : joints)
  {
    if (!points.count(entry.second.a) || (entry.second.b && !points.count(entry.second.b)))
    {
      return false;
    }
  }
  return true;
}

QJsonObject ConnectionRunMap::json() const
{
  QJsonArray ps, js;
  for (const auto& entry : points)
  {
    const auto& p = entry.second;
    ps.append(QJsonObject{{"id",p.id},{"name",QString::fromStdString(p.name)},{"ref",p.ref},{"rot",p.rot},
      {"history",p.history},{"generated_ref",p.generated_ref},{"generated_rot",p.generated_rot},{"body",p.body},{"body_name",QString::fromStdString(p.body_name)},{"position",QJsonArray{p.position[0],p.position[1],p.position[2]}}});
  }
  for (const auto& entry : joints)
  {
    const auto& j = entry.second;
    js.append(QJsonObject{{"id",j.id},{"name",QString::fromStdString(j.name)},{"type",QString::fromStdString(j.type)},
      {"a",j.a},{"b",j.b},{"history",j.history},{"helper_nodes",j.helper_nodes},{"helper_elements",j.helper_elements},{"guide",j.guide},
      {"guide_position",QJsonArray{j.guide_position[0],j.guide_position[1],j.guide_position[2]}},{"axis",QJsonArray{j.axis[0],j.axis[1],j.axis[2]}}});
  }
  QJsonArray attachments;
  for (const auto& entry : distributed)
  {
    const auto& a = entry.second;
    QJsonArray weights;
    for (const auto& w : a.weights)
    {
      weights.append(QJsonArray{w.first,w.second});
    }
    attachments.append(QJsonObject{{"id",a.id},{"ref",a.ref},{"point",a.point},{"name",QString::fromStdString(a.name)},
      {"rp_name",QString::fromStdString(a.point_name)},{"position",QJsonArray{a.position[0],a.position[1],a.position[2]}},{"weights",weights},{"prescribed",a.prescribed}});
  }
  QJsonArray definitions;
  for (const auto& entry : discrete)
  {
    const auto& d = entry.second;
    QJsonArray steps;
    for (const auto& step : d.history)
    {
      steps.append(QJsonArray{step.first,step.second});
    }
    definitions.append(QJsonObject{{"id",d.id},{"element",d.element},{"name",QString::fromStdString(d.name)},
      {"type",QString::fromStdString(d.type)},{"a",d.a},{"b",d.b},{"direction",d.direction},{"value",d.value},
      {"position_a",QJsonArray{d.position_a[0],d.position_a[1],d.position_a[2]}},
      {"position_b",QJsonArray{d.position_b[0],d.position_b[1],d.position_b[2]}},{"history",steps}});
  }
  return {{"points",ps},{"joints",js},{"distributed",attachments},{"discrete",definitions}};
}

bool ConnectionRunMap::restore(const QJsonObject& object, std::string& error)
{
  ConnectionRunMap result;
  if (!object.isEmpty() && (!object.value("points").isArray() || !object.value("joints").isArray()))
  {
    error = "Invalid connection run mapping.";
    return false;
  }
  for (const auto& item : object.value("points").toArray())
  {
    if (!point(item.toObject(),result))
    {
      error = "Invalid saved reference point.";
      return false;
    }
  }
  for (const auto& item : object.value("joints").toArray())
  {
    if (!joint(item.toObject(),result))
    {
      error = "Invalid saved joint.";
      return false;
    }
  }
  for (const auto& item : object.value("distributed").toArray())
  {
    if (!attachment(item.toObject(),result))
    {
      error = "Invalid saved distributing attachment.";
      return false;
    }
  }
  for (const auto& item : object.value("discrete").toArray())
  {
    if (!read_discrete(item.toObject(),result))
    {
      error = "Invalid archived point mass or spring.";
      return false;
    }
  }
  if (!result.valid())
  {
    error = "Invalid saved joint endpoint or control node.";
    return false;
  }
  *this = std::move(result);
  return true;
}
