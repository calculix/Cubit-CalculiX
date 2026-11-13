#pragma once
#include "CoreDrawTypes.hpp"
#include "Framework/Drawing/DrawingGeometry.hpp"

class CoreDrawEntityGeometry
{
public:
  CoreDrawEntityGeometry();
  static std::string color(int index);
  // Load/BC symbols only. Reads definitions through existing workspace interfaces.
  // No scene submission or selection changes. Failure leaves output unchanged.
  static bool build(DrawEntityKey entity, double symbolSize, DrawingGeometry& output, std::string& error);
};
