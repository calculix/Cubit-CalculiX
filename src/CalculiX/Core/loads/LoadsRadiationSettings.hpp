#ifndef CALCULIX_LOADSRADIATIONSETTINGS_HPP
#define CALCULIX_LOADSRADIATIONSETTINGS_HPP

#include <string>

struct LoadsRadiationSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string amplitude;
  std::string timeDelay;
  std::string operation;
  std::string sideset;
  std::string temperature;
  std::string coefficient;
  std::string secondaryAmplitude;
  std::string secondaryTimeDelay;
  std::string cavity;
  std::string cavityRadiation;
};

#endif
