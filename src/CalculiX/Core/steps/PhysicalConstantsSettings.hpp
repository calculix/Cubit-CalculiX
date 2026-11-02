#ifndef CALCULIX_PHYSICALCONSTANTSSETTINGS_HPP
#define CALCULIX_PHYSICALCONSTANTSSETTINGS_HPP

#include <string>

struct PhysicalConstantsSettings
{
  int id = -1;
  int type = 0;
  std::string absoluteZero;
  std::string stefanBoltzmann;
  std::string newtonGravity;
};

#endif
