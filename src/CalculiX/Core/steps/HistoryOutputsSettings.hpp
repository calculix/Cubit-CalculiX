#ifndef CALCULIX_HISTORYOUTPUTSSETTINGS_HPP
#define CALCULIX_HISTORYOUTPUTSSETTINGS_HPP

#include <string>
#include <vector>

struct HistoryOutputsSettings
{
  std::vector<std::string> parameters;
  std::vector<std::string> enabledKeys;
  int id = -1;
  int type = 0;
  std::string name;
};

#endif
