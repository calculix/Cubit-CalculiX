#ifndef CALCULIX_LOADSTRAJECTORYSETTINGS_HPP
#define CALCULIX_LOADSTRAJECTORYSETTINGS_HPP

#include <string>
#include <vector>

struct LoadsTrajectorySettings
{
  std::vector<std::vector<double>> samples;
  int id = -1;
  int type = 0;
  std::string name;
  std::string loadType;
  std::string curve;
  std::string vertex;
  std::string surfaces;
  std::string direction;
  std::string timeBegin;
  std::string timeEnd;
  std::string operation;
  std::string modelChange;
};

#endif
