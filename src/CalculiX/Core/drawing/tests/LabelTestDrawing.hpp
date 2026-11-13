#pragma once
#include "Framework/Interfaces/DrawingInterface.hpp"

class LabelTestDrawing final : public DrawingInterface
{
public:
  DrawingFrame frame;
  DrawingFrame capturedFrame;
  bool captureSucceeds = true;
  bool activated = false;
  bool submit(const std::string& workspace, DrawingFrame input, std::string& error) override;
  bool capture(const DrawingFrame& input, const DrawingCaptureRequest&, QByteArray& png, std::string& error) override;
  void set_pick_callback(const std::string&, DrawingPickCallback) override;
  bool activate_workspace(const std::string&, std::string&) override;
  bool picking_active(const std::string&) const override;
  bool pick(const std::string&, int, double, double, DrawingSelection&, std::string&) override;
  void clear_hover(const std::string&) noexcept override;
  void clear(const std::string&) noexcept override;
};
