#ifndef CALCULIX_CUSTOMLINESSETTINGS_HPP
#define CALCULIX_CUSTOMLINESSETTINGS_HPP

#include <string>

struct CustomLinesSettings
{
  int id = -1;
  int type = 0;
  std::string name;
  std::string position;
  std::string keyword;
  std::string keywordId;
  std::string text;
};

#endif
