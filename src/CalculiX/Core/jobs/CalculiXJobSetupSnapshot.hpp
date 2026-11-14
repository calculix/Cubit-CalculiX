#pragma once
#include "CalculiXJobSetupScene.hpp"
#include "Framework/Drawing/DrawingCaptureCamera.hpp"

class CalculiXJobSetupSnapshot
{
public:
  CalculiXJobSetupSnapshot();
  std::string id;
  std::string fingerprint;
  double symbolSize = 0;
  DrawingGeometry model;
  DrawingCaptureCamera camera;
  CalculiXJobSetupScene modelDefinitions;
  std::vector<CalculiXJobSetupScene> steps;
};
