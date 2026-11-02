#ifndef CALCULIX_INITIALCONDITIONSSETTINGS_HPP
#define CALCULIX_INITIALCONDITIONSSETTINGS_HPP

#include <string>
#include <vector>

struct InitialConditionsSettings
{
  std::vector<std::vector<double>> blockStress;
  std::vector<std::vector<double>> elementStress;
  int id = -1;
  int type = 0;
  std::string bcId;
  std::string conditionType;
};

#endif
