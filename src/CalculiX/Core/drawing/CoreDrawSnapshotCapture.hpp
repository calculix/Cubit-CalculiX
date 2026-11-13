#pragma once
#include "CalculiXCaptureRequest.hpp"
#include "Core/jobs/CalculiXJobSetupSnapshot.hpp"
#include "Framework/Drawing/DrawingTypes.hpp"

class CoreDrawSnapshotCapture
{
public:
  CoreDrawSnapshotCapture();
  // Pure scene composition of a validated immutable snapshot. No Cubit mesh
  // or live CalculiX condition queries. Failure preserves the output frame.
  static bool buildFrame(const CalculiXJobSetupSnapshot& snapshot, const CalculiXCaptureRequest& request,
    DrawingFrame& output, std::string& error);
};
