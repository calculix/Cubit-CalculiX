#pragma once
#include "CalculiXCaptureRequest.hpp"
#include <QByteArray>
class CalculiXJobSetupSnapshot;

class CalculiXCaptureInterface
{
public:
  CalculiXCaptureInterface();
  // Current model only; stepId is a model step ID, not a result increment.
  // Host thread only. Does not activate a workspace or change live drawing state.
  // Failure leaves png unchanged; success clears error.
  bool render(const CalculiXCaptureRequest& request, QByteArray& png, std::string& error);
  bool render_snapshot(const CalculiXJobSetupSnapshot& snapshot, const CalculiXCaptureRequest& request, QByteArray& png, std::string& error);
};
