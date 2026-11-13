#include "LabelTestDrawing.hpp"
#include <utility>

bool LabelTestDrawing::submit(const std::string&, DrawingFrame input, std::string& error)
{
  frame = std::move(input);
  error.clear();
  return true;
}
void LabelTestDrawing::set_pick_callback(const std::string&, DrawingPickCallback)
{
}
bool LabelTestDrawing::activate_workspace(const std::string&, std::string& error)
{
  activated = true;
  error.clear();
  return true;
}

bool LabelTestDrawing::capture(const DrawingFrame& input, const DrawingCaptureRequest&, QByteArray& png, std::string& error)
{
  if (!captureSucceeds)
  {
    error = "Capture fixture failure.";
    return false;
  }
  capturedFrame = input;
  png = "fixture image bytes";
  error.clear();
  return true;
}
bool LabelTestDrawing::picking_active(const std::string&) const
{
  return true;
}
bool LabelTestDrawing::pick(const std::string&, int, double, double, DrawingSelection&, std::string& error)
{
  error.clear();
  return true;
}
void LabelTestDrawing::clear_hover(const std::string&) noexcept
{
}
void LabelTestDrawing::clear(const std::string&) noexcept
{
  frame = {};
}
