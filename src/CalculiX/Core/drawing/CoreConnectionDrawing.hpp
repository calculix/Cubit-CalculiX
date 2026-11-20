#pragma once
#include "CoreDrawTypes.hpp"
#include "Framework/Drawing/DrawingGeometry.hpp"
class CoreConnections;

class CoreConnectionDrawing
{
public:
  static bool resolve(const CoreConnections& connections, DrawEntityKey key, double size,
    DrawingGeometry& geometry);
};
