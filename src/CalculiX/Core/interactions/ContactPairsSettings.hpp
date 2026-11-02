#ifndef CALCULIX_CONTACTPAIRSSETTINGS_HPP
#define CALCULIX_CONTACTPAIRSSETTINGS_HPP

#include <string>

struct ContactPairsSettings
{
  int id = -1;
  int type = 0;
  std::string surfaceInteraction;
  std::string contactType;
  std::string master;
  std::string slave;
  std::string adjust;
  std::string adjustNodeset;
};

#endif
