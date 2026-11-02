#ifndef CALCULIX_ORIENTATIONSSETTINGS_HPP
#define CALCULIX_ORIENTATIONSSETTINGS_HPP

#include <string>

struct OrientationsSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string system;
  std::string a;
  std::string b;
  std::string axis;
  std::string angle;
};

#endif
