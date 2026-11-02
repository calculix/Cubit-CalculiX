#ifndef CALCULIX_SECTIONSSETTINGS_HPP
#define CALCULIX_SECTIONSSETTINGS_HPP

#include <string>

struct SectionsSettings
{
  int id = -1;
  int type = 0;
  std::string block;
  std::string material;
  std::string orientation;
  std::string thickness;
  std::string offset;
  std::string beamType;
  std::string parameter1;
  std::string parameter2;
  std::string parameter3;
  std::string parameter4;
  std::string parameter5;
  std::string parameter6;
  std::string x;
  std::string y;
  std::string z;
  std::string offset1;
  std::string offset2;
};

#endif
