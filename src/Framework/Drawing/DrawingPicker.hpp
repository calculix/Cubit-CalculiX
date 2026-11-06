#ifndef CUBITSIM_DRAWINGPICKER_HPP
#define CUBITSIM_DRAWINGPICKER_HPP

#include "DrawingPickData.hpp"
#include "DrawingHover.hpp"
#include "SVPickUtil.hpp"
#include "SVSelection.hpp"
#include <functional>
#include <memory>

class DrawingPicker : public GPicker, public GPicked::PickCallbackNow
{
public:
  DrawingPicker();
  ~DrawingPicker() override;
  bool activate(std::string& error);
  void deactivate() noexcept;
  void shutdown() noexcept;
  void clearHover() noexcept;
  void setHoverColor(const std::string& color);
  bool pickSymbol(int viewId, double xFraction, double yFraction, DrawingSelection& selection, std::string& error);
  bool active() const;
  bool ownsPickMode() const;
  void updateSnapshot(DrawingPickSnapshot snapshot);
  void setCallback(std::function<void(const DrawingSelection&)> callback);
  void compute_pick(float x, float y, PickViewInfo& info) override;
  void compute_pre_select(float x, float y, PickViewInfo& info) override;
  void compute_pick_by_bounds(PickBounds* bounds, PickViewInfo& info) override;
  void notify_picked(int num, GPicked::PickedEntity const* const* entities, float const* const* points, bool addOrRemove) override;
  void notify_pre_select(GPicked::PickedEntity const* entity, const float point[3], bool addOrRemove) override;

private:
  struct DispatchState
  {
    std::function<void(const DrawingSelection&)> callback;
    std::size_t revision = 0;
    std::size_t serial = 0;
    bool active = false;
  };
  void clearOwnedSelections();
  void dispatchSelection(const DrawingSelection& selection);
  DrawingHover hover;
  std::string hoverColor = "red";
  std::size_t hoverHandle = 0;
  std::size_t hoverRevision = 0;
  DrawingPickSnapshot snapshot;
  std::shared_ptr<DispatchState> dispatch;
  std::string previousType;
  bool registered = false;
  bool internalUpdate = false;
};

#endif
