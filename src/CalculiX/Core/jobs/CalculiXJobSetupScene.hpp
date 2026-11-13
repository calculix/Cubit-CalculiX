#pragma once
#include "Core/drawing/DrawLabelData.hpp"
#include "Framework/Drawing/DrawingGeometry.hpp"

class CalculiXJobSetupScene
{
public:
  CalculiXJobSetupScene();
  int stepId = -1;
  int ordinal = 0;
  std::string name;
  std::map<DrawEntityKey, DrawingGeometry> symbols;
  std::map<DrawEntityKey, DrawLabelData> labels;
  bool available = false;
  std::string unavailableReason;
};
