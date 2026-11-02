#ifndef CALCULIX_STEPSSETTINGS_HPP
#define CALCULIX_STEPSSETTINGS_HPP

#include <string>
#include <vector>

struct StepsSettings
{
  std::vector<std::string> parameters;
  std::vector<std::string> procedure;
  int id = -1;
  int type = 0;
  std::string name;
};

#endif
