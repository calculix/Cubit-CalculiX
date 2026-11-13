#pragma once
#include "CalculiXJobSetupScene.hpp"

class CalculiXJobSetupSnapshot
{
public:
  CalculiXJobSetupSnapshot();
  std::string id;
  std::string fingerprint;
  double symbolSize = 0;
  DrawingGeometry model;
  CalculiXJobSetupScene modelDefinitions;
  std::vector<CalculiXJobSetupScene> steps;
};
