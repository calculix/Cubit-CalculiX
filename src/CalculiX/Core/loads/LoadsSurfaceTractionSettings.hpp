#ifndef CALCULIX_LOADSSURFACETRACTIONSETTINGS_HPP
#define CALCULIX_LOADSSURFACETRACTIONSETTINGS_HPP

#include <string>

struct LoadsSurfaceTractionSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string sideset;
  std::string component1;
  std::string component2;
  std::string component3;
};

#endif
