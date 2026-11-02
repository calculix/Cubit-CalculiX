#ifndef CALCULIX_JOBSSETTINGS_HPP
#define CALCULIX_JOBSSETTINGS_HPP

#include <string>

struct JobsSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string filepath;
};

#endif
