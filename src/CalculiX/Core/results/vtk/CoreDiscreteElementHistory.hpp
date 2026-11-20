#pragma once

#include "Core/connections/ConnectionRunMap.hpp"
#include <map>
#include <string>
#include <utility>
#include <vector>

class CoreDiscreteElementHistory
{
public:
  using Sample = std::array<double,3>;
  using Columns = std::vector<std::pair<std::string,double>>;
  static Columns evaluate(const DiscreteResultBinding& feature, int step, const std::map<int,Sample>& samples);
};
