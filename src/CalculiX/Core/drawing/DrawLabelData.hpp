#pragma once
#include "CoreDrawTypes.hpp"
#include <array>
#include <string>
#include <utility>
#include <vector>

class DrawLabelData
{
public:
  DrawLabelData();
  DrawEntityKey entity;
  int stepId = -1;
  double symbolSize = 1.0;
  std::string typeName;
  std::string name;
  std::string amplitude;
  std::string secondaryAmplitude;
  std::vector<std::pair<std::string, std::string>> values;
  std::vector<std::array<double, 3>> anchors;
};
