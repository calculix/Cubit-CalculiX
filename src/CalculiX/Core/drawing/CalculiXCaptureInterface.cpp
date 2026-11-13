#include "CalculiXCaptureInterface.hpp"
#include "CoreDraw.hpp"
#include "Core/loadCalculiXCore.hpp"

CalculiXCaptureInterface::CalculiXCaptureInterface() = default;

bool CalculiXCaptureInterface::render(const CalculiXCaptureRequest& request, QByteArray& png, std::string& error)
{
  if (ccx_core.draw == nullptr)
  {
    error = "CalculiX drawing is unavailable.";
    return false;
  }
  return ccx_core.draw->capturePNG(request, png, error);
}

bool CalculiXCaptureInterface::render_snapshot(const CalculiXJobSetupSnapshot& snapshot, const CalculiXCaptureRequest& request, QByteArray& png, std::string& error)
{
  if (ccx_core.draw == nullptr)
  {
    error = "CalculiX drawing is unavailable.";
    return false;
  }
  return ccx_core.draw->captureSnapshotPNG(snapshot, request, png, error);
}
