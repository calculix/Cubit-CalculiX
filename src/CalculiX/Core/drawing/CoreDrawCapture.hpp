#pragma once
#include "CalculiXCaptureRequest.hpp"
#include "Framework/Drawing/DrawingTypes.hpp"
class CalculiXCore;

class CoreDrawCapture
{
public:
  CoreDrawCapture();
  // Builds only local geometry. The live scene, camera and selection are untouched.
  // Symbol length is 10% of the largest model extent, in world coordinates.
  static bool buildFrame(CalculiXCore& core, const CalculiXCaptureRequest& request,
    DrawingFrame& output, std::string& error);
private:
  static bool validate(const CalculiXCaptureRequest& request, std::string& error);
};
