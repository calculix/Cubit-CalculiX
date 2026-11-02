#ifndef CALCULIX_SURFACEINTERACTIONSSETTINGS_HPP
#define CALCULIX_SURFACEINTERACTIONSSETTINGS_HPP

#include <string>
#include <vector>

struct SurfaceInteractionsSettings
{
  std::vector<std::vector<double>> pressureOverclosure;
  std::vector<std::vector<double>> conductance;
  int id = -1;
  int type = 0;
  std::string name;
  std::string behavior;
  std::string c0;
  std::string p0;
  std::string slope;
  std::string sigma;
  std::string linearC0;
  std::string tiedSlope;
  std::string conversion;
  std::string weight;
  std::string velocity;
  std::string mu;
  std::string lambdaValue;
};

#endif
