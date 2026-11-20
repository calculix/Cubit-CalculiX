#include "CoreDiscreteElementHistory.hpp"
#include <cmath>

CoreDiscreteElementHistory::Columns CoreDiscreteElementHistory::evaluate(const DiscreteResultBinding& feature,
  int step, const std::map<int,Sample>& samples)
{
  const auto active = feature.history.find(step);
  const auto first = samples.find(feature.a);
  if (active == feature.history.end() || first == samples.end())
  {
    return {};
  }
  const auto& u = first->second;
  if (feature.type == "MASS")
  {
    return {{"Displacement X",u[0]},{"Displacement Y",u[1]},{"Displacement Z",u[2]}};
  }
  if (feature.type == "SPRING1")
  {
    const double travel = u.at(feature.direction-1);
    return {{"Travel",travel},{"Derived restoring force",-feature.value*travel}};
  }
  const auto second = samples.find(feature.b);
  if (second == samples.end())
  {
    return {};
  }
  Sample original, relative, current;
  for (int i = 0; i < 3; ++i)
  {
    original[i] = feature.position_b[i] - feature.position_a[i];
    relative[i] = second->second[i] - u[i];
    current[i] = original[i] + relative[i];
  }
  const double length = std::hypot(original[0],original[1],original[2]);
  if (!(length > 0))
  {
    return {};
  }
  double extension = 0;
  if (active->second)
  {
    extension = std::hypot(current[0],current[1],current[2]) - length;
  }
  else
  {
    for (int i = 0; i < 3; ++i)
    {
      extension += original[i]/length * relative[i];
    }
  }
  return {{"Extension",extension},{"Derived axial force",feature.value*extension}};
}
