#ifndef CALCULIX_LOADSGRAVITYSETTINGS_HPP
#define CALCULIX_LOADSGRAVITYSETTINGS_HPP

#include <string>

struct LoadsGravitySettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string block;
  std::string magnitude;
  std::string direction;
};

#endif
