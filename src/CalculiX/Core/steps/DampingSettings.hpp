#ifndef CALCULIX_DAMPINGSETTINGS_HPP
#define CALCULIX_DAMPINGSETTINGS_HPP

#include <string>

struct DampingSettings
{
  int id = -1;
  int type = 0;
  std::string alpha;
  std::string beta;
};

#endif
