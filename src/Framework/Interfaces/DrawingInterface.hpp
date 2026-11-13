#ifndef CUBITSIM_DRAWINGINTERFACE_HPP
#define CUBITSIM_DRAWINGINTERFACE_HPP

#include "Framework/Drawing/DrawingTypes.hpp"
#include "Framework/Drawing/DrawingCaptureRequest.hpp"
#include <QByteArray>

class DrawingInterface
{
public:
  virtual ~DrawingInterface() = default;
  // GUI-thread operations. submit copies the frame and validates ownership.
  // Coordinates are finite world coordinates; rendering never resolves solver data.
  virtual bool submit(const std::string& workspace, DrawingFrame frame, std::string& error) = 0;
  // GUI-thread operation. Uses a private scene/window; no activation, submission
  // or mutation of the live camera, selection or frame. Failure preserves png.
  virtual bool capture(const DrawingFrame& frame, const DrawingCaptureRequest& request, QByteArray& png, std::string& error) = 0;
  virtual void set_pick_callback(const std::string& workspace, DrawingPickCallback callback) = 0;
  // Called by the framework on workspace switches, including the native workspace.
  virtual bool activate_workspace(const std::string& workspace, std::string& error) = 0;
  virtual bool picking_active(const std::string& workspace) const = 0;
  virtual bool pick(const std::string& workspace, int viewId, double x, double y, DrawingSelection& selection, std::string& error) = 0;
  virtual void clear_hover(const std::string& workspace) noexcept = 0;
  // Also invalidates queued pick callbacks for this workspace.
  virtual void clear(const std::string& workspace) noexcept = 0;
};
#endif
