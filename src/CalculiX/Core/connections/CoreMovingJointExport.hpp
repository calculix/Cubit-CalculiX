#pragma once
#include <array>
#include <iosfwd>
#include <set>
#include <string>
#include <utility>
#include <QJsonArray>
#include <QJsonObject>

struct ConnectionJoint;
class CoreExportNodeAllocator;
class CoreMovingJointExport
{
public:
  using Position = std::array<double,3>;
  bool add_markers(bool guide_side, const Position& origin, const Position& axis, double span,
    CoreExportNodeAllocator& allocator, std::ostream& nodes, std::ostream& members, std::string& error);
  bool append(const ConnectionJoint& joint, int a, int b, CoreExportNodeAllocator& allocator,
    std::ostream& nodes, std::ostream& elements, std::ostream& constraints,
    std::set<std::pair<int,int>>& dependent, QJsonObject& binding, std::string& error);
private:
  int guide = 0;
  int tip = 0;
  std::array<int,2> planes{};
  Position guide_position{};
  Position tip_position{};
  Position origin_b{};
  QJsonArray helpers;
};
