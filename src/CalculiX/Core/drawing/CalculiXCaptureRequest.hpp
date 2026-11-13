#pragma once
#include "CoreDrawTypes.hpp"
#include "Framework/Drawing/DrawingCaptureRequest.hpp"

enum class CalculiXCaptureContent
{
  Model, Setup, Loads, BoundaryConditions
};

class CalculiXCaptureRequest
{
public:
  CalculiXCaptureRequest();
  CalculiXCaptureContent content;
  DrawScope scope;
  DrawLabelSettings labels;
  DrawingCaptureRequest image;
};
