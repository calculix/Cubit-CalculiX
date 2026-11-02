#ifndef CALCULIX_BCSTEMPERATURESSETTINGS_HPP
#define CALCULIX_BCSTEMPERATURESSETTINGS_HPP

#include <string>

struct BCsTemperaturesSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string nodeset;
  std::string magnitude;
  std::string keyword;
};

#endif
