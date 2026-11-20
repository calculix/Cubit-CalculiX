#pragma once
#include <array>
#include <set>
#include <ostream>
#include <string>

struct ConnectionJoint;
class CoreJointExport
{
public:
  static bool append(const ConnectionJoint& joint, const std::array<int, 2>& side_a,
    const std::array<int, 2>& side_b, const std::array<double, 3>& axis, int guide,
    std::set<std::pair<int,int>>& dependent, std::ostream& output, std::string& error);
};
