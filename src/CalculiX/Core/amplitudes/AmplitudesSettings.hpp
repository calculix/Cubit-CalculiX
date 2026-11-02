#ifndef CALCULIX_AMPLITUDESSETTINGS_HPP
#define CALCULIX_AMPLITUDESSETTINGS_HPP

#include <string>
#include <vector>

struct AmplitudesSettings
{
  std::vector<std::vector<double>> points;
  int id = -1;
  int type = 0;
  std::string name;
  std::string shiftX;
  std::string shiftY;
  std::string totalTime;
};

#endif
