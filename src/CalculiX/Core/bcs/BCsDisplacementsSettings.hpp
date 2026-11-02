#ifndef CALCULIX_BCSDISPLACEMENTSSETTINGS_HPP
#define CALCULIX_BCSDISPLACEMENTSSETTINGS_HPP

#include <string>

struct BCsDisplacementsSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string nodeset;
  std::string component1;
  std::string component2;
  std::string component3;
  std::string component4;
  std::string component5;
  std::string component6;
};

#endif
