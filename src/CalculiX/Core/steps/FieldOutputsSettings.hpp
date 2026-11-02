#ifndef CALCULIX_FIELDOUTPUTSSETTINGS_HPP
#define CALCULIX_FIELDOUTPUTSSETTINGS_HPP

#include <string>
#include <vector>

struct FieldOutputsSettings
{
  std::vector<std::string> parameters;
  std::vector<std::string> enabledKeys;
  int id = -1;
  int type = 0;
  std::string name;
};

#endif
