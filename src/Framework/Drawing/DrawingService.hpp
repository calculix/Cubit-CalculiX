#ifndef CUBITSIM_DRAWINGSERVICE_HPP
#define CUBITSIM_DRAWINGSERVICE_HPP

#include "DrawingExport.hpp"
#include "DrawingGraphics.hpp"
#include "DrawingPicker.hpp"
#include "Framework/Interfaces/DrawingInterface.hpp"

class CUBITSIM_DRAWING_EXPORT DrawingService : public DrawingInterface
{
public:
  DrawingService();
  ~DrawingService() override;
  bool submit(const std::string& workspace, DrawingFrame frame, std::string& error) override;
  void set_pick_callback(const std::string& workspace, DrawingPickCallback callback) override;
  bool activate_workspace(const std::string& workspace, std::string& error) override;
  bool picking_active(const std::string& workspace) const override;
  bool pick(const std::string& workspace, int viewId, double x, double y, DrawingSelection& selection, std::string& error) override;
  void clear_hover(const std::string& workspace) noexcept override;
  void clear(const std::string& workspace) noexcept override;
private:
  bool render(std::string& error);
  std::map<std::string, DrawingFrame> frames;
  std::map<std::string, DrawingPickCallback> callbacks;
  std::string activeWorkspace = "cubit";
  std::size_t revision = 0;
  DrawingGraphics graphics;
  DrawingPicker picker;
};
#endif
