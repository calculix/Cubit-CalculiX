#ifndef CALCULIX_LOADSCENTRIFUGALSETTINGS_HPP
#define CALCULIX_LOADSCENTRIFUGALSETTINGS_HPP

#include <string>

struct LoadsCentrifugalSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string block;
  std::string magnitude;
  std::string coordinate;
  std::string direction;
};

#endif
