#ifndef CALCULIX_LOADSBODYHEATFLUXSETTINGS_HPP
#define CALCULIX_LOADSBODYHEATFLUXSETTINGS_HPP

#include <string>

struct LoadsBodyHeatfluxSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string magnitude;
  std::string entityType;
  std::string ids;
};

#endif
