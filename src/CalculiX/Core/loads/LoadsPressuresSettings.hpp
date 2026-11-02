#ifndef CALCULIX_LOADSPRESSURESSETTINGS_HPP
#define CALCULIX_LOADSPRESSURESSETTINGS_HPP

#include <string>

struct LoadsPressuresSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string sideset;
  std::string magnitude;
};

#endif
