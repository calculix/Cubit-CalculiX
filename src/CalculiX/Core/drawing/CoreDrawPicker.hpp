#ifndef COREDRAWPICKER_HPP
#define COREDRAWPICKER_HPP

#include "CoreDrawPickData.hpp"
#include "CoreDrawHover.hpp"
#include "SVPickUtil.hpp"
#include "SVSelection.hpp"
#include <functional>
#include <memory>

class CoreDrawPicker : public GPicker, public GPicked::PickCallbackNow
{
public:
  CoreDrawPicker();
  ~CoreDrawPicker() override;
  bool activate(std::string& error);
  void deactivate() noexcept;
  void shutdown() noexcept;
  void clearHover() noexcept;
  void setHoverColor(const std::string& color);
  bool pickSymbol(int viewId, double xFraction, double yFraction, CalculiXDrawSelection& selection, std::string& error);
  bool active() const;
  bool ownsPickMode() const;
  void updateSnapshot(DrawPickSnapshot snapshot);
  void setCallback(std::function<void(const CalculiXDrawSelection&)> callback);
  void compute_pick(float x, float y, PickViewInfo& info) override;
  void compute_pre_select(float x, float y, PickViewInfo& info) override;
  void compute_pick_by_bounds(PickBounds* bounds, PickViewInfo& info) override;
  void notify_picked(int num, GPicked::PickedEntity const* const* entities, float const* const* points, bool addOrRemove) override;
  void notify_pre_select(GPicked::PickedEntity const* entity, const float point[3], bool addOrRemove) override;

private:
  struct DispatchState
  {
    std::function<void(const CalculiXDrawSelection&)> callback;
    std::size_t revision = 0;
    std::size_t serial = 0;
    bool active = false;
  };
  void clearOwnedSelections();
  void dispatchSelection(const CalculiXDrawSelection& selection);
  CoreDrawHover hover;
  std::string hoverColor = "red";
  std::size_t hoverHandle = 0;
  std::size_t hoverRevision = 0;
  DrawPickSnapshot snapshot;
  std::shared_ptr<DispatchState> dispatch;
  std::string previousType;
  bool registered = false;
  bool internalUpdate = false;
};

#endif
