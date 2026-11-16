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
  // Model, symbols, label anchors, symbolSize and camera all use the run's
  // recorded geometryLengthUnit. Mesh coordinates are converted from the deck
  // during construction; no duplicate solver-coordinate mesh is retained here.
  DrawingGeometry model;
  DrawingCaptureCamera camera;
  CalculiXJobSetupScene modelDefinitions;
  std::vector<CalculiXJobSetupScene> steps;
};
