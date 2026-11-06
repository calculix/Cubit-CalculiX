#include "DrawingPicker.hpp"
#include "DrawingHitTest.hpp"
#include "GWindowFactory.hpp"
#include "GView.hpp"
#include "CubitMessage.hpp"
#include <QCoreApplication>
#include <QTimer>
#include <QScopedValueRollback>
#include <cstring>
#include <cmath>
#include <exception>
#include <utility>
#include <vtkMatrix4x4.h>
#include <vtkDataSet.h>

namespace
{
const char SymbolPickType[] = "cubitsim_symbol";

// Cubit 2026.11 passes a render snapshot rather than a renderer. Views with
// identical visible datasets and projection are equivalent for this hit test.
bool matchesView(GView& view, const PickViewInfo& info)
{
  auto* renderer = view.get_renderer();
  if (renderer == nullptr)
  {
    return false;
  }
  const auto candidate = SVPickUtil::pick_info(renderer);
  if (candidate.data != info.data || candidate.winsize[0] != info.winsize[0] || candidate.winsize[1] != info.winsize[1] ||
    !candidate.world_to_view || !info.world_to_view)
  {
    return false;
  }
  for (int i = 0; i < 4; ++i)
  {
    if (candidate.viewport[i] != info.viewport[i])
    {
      return false;
    }
    for (int j = 0; j < 4; ++j)
    {
      const double difference = candidate.world_to_view->GetElement(i, j) - info.world_to_view->GetElement(i, j);
      if (!std::isfinite(difference) || std::abs(difference) > 1e-12)
      {
        return false;
      }
    }
  }
  return true;
}

class PickedDrawEntity : public GPicked::PickedEntity
{
public:
  PickedDrawEntity(const DrawingPickHit& hit, std::size_t revision)
    : owner(hit.owner), revision(revision), pickHandle(hit.handle), caption(hit.owner.entity.kind + " " + std::to_string(hit.owner.entity.id))
  {
  }
  const char* type() const override { return SymbolPickType; }
  int id() const override { return owner.entity.id; }
  const char* name() const override { return caption.c_str(); }
  std::size_t handle() const override { return pickHandle; }
  DrawingSelection owner;
  std::size_t revision;
private:
  std::size_t pickHandle;
  std::string caption;
};
}

DrawingPicker::DrawingPicker() : dispatch(std::make_shared<DispatchState>())
{
}

DrawingPicker::~DrawingPicker()
{
  shutdown();
}

bool DrawingPicker::ownsPickMode() const
{
  return registered && SVSelection::get_picker() == this;
}

bool DrawingPicker::active() const
{
  return dispatch->active;
}

bool DrawingPicker::activate(std::string& error)
{
  error.clear();
  if (active() && ownsPickMode())
  {
    return true;
  }
  const char* previous = SVSelection::get_pick_type();
  if (previous == nullptr || std::strcmp(previous, SymbolPickType) == 0)
  {
    error = "No previous Cubit pick mode is available to restore.";
    return false;
  }
  previousType = previous;
  if (!registered)
  {
    if (!SVSelection::register_picker(SymbolPickType, this))
    {
      error = "Cubit could not register cubit-sim symbol picking.";
      return false;
    }
    SVSelection::add_pick_observer(this);
    SVSelection::add_pre_selection_observer(this);
    registered = true;
  }
  SVSelection::set_pick_type(SymbolPickType, true);
  dispatch->active = ownsPickMode();
  if (!dispatch->active)
  {
    error = "Cubit could not activate cubit-sim symbol picking.";
  }
  return dispatch->active;
}

void DrawingPicker::clearOwnedSelections()
{
  const QScopedValueRollback<bool> guard(internalUpdate, true);
  if (dynamic_cast<const PickedDrawEntity*>(SVSelection::get_current_pre_selection()) != nullptr)
  {
    SVSelection::set_pre_selection(EntitySelection());
  }
  clear_selection_buffer();
  const EntitySelection* picked = nullptr;
  int count = 0;
  SVSelection::get_picked_list(picked, count);
  std::vector<GPicked::PickedEntity*> owned;
  for (int index = 0; index < count; ++index)
  {
    if (dynamic_cast<const PickedDrawEntity*>(picked[index].mEntity) != nullptr)
    {
      owned.push_back(picked[index].mEntity);
    }
  }
  if (!owned.empty())
  {
    SVSelection::remove_from_picked_list(owned.size(), owned.data());
  }
  const EntitySelection* possible = nullptr;
  SVSelection::get_possible_selections(possible, count);
  owned.clear();
  for (int index = 0; index < count; ++index)
  {
    if (dynamic_cast<const PickedDrawEntity*>(possible[index].mEntity) != nullptr)
    {
      owned.push_back(possible[index].mEntity);
    }
  }
  for (auto* entity : owned)
  {
    SVSelection::remove_from_possible_selections_list(entity);
  }
}

void DrawingPicker::deactivate() noexcept
{
  dispatch->active = false;
  clearHover();
  ++dispatch->serial;
  if (!registered)
  {
    return;
  }
  try
  {
    clearOwnedSelections();
    if (ownsPickMode())
    {
      const int count = SVSelection::get_number_pick_types();
      std::vector<const char*> types(static_cast<std::size_t>(count));
      SVSelection::get_pick_types(types.data());
      const char* fallback = nullptr;
      const char* restore = nullptr;
      for (const char* type : types)
      {
        if (type != nullptr && std::strcmp(type, SymbolPickType) != 0)
        {
          if (fallback == nullptr)
          {
            fallback = type;
          }
          if (previousType == type)
          {
            restore = type;
            break;
          }
        }
      }
      if (restore != nullptr || fallback != nullptr)
      {
        SVSelection::set_pick_type(restore == nullptr ? fallback : restore, true);
      }
    }
  }
  catch (const std::exception& error)
  {
    PRINT_WARNING("cubit-sim pick-mode restoration failed: %s\n", error.what());
  }
}

void DrawingPicker::shutdown() noexcept
{
  deactivate();
  dispatch->callback = {};
  if (registered)
  {
    SVSelection::remove_pre_selection_observer(this);
    SVSelection::remove_pick_observer(this);
    SVSelection::unregister_picker(SymbolPickType, this);
    registered = false;
  }
  hover.detach();
  snapshot.entities.clear();
}

void DrawingPicker::updateSnapshot(DrawingPickSnapshot data)
{
  clearHover();
  if (registered)
  {
    clearOwnedSelections();
  }
  snapshot = std::move(data);
  dispatch->revision = snapshot.revision;
  ++dispatch->serial;
}

void DrawingPicker::setCallback(std::function<void(const DrawingSelection&)> callback)
{
  dispatch->callback = std::move(callback);
  ++dispatch->serial;
}

void DrawingPicker::compute_pick(float x, float y, PickViewInfo& info)
{
  clear_selection_buffer();
  if (!active() || !ownsPickMode())
  {
    return;
  }
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    return;
  }
  try
  {
    const QScopedValueRollback<bool> guard(internalUpdate, true);
    for (const auto& view : factory->views())
    {
      if (view != nullptr && matchesView(*view, info))
      {
        DrawingPickHit hit;
        DrawingSelection result;
        if (DrawingHitTest().pick(*view, x, y, snapshot, hit))
        {
          auto* entity = new PickedDrawEntity(hit, snapshot.revision);
          const std::array<float, 3> point = {{static_cast<float>(hit.world[0]), static_cast<float>(hit.world[1]), static_cast<float>(hit.world[2])}};
          add_to_selection_buffer(entity, point, static_cast<float>(hit.display[2]), true);
          entity->Delete();
          result = hit.owner;
        }
        // Also dispatch an empty-space click when Cubit has no native pick-list change.
        dispatchSelection(result);
        return;
      }
    }
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("cubit-sim symbol picking failed: %s\n", error.what());
  }
}

void DrawingPicker::clearHover() noexcept
{
  hoverHandle = 0;
  hoverRevision = 0;
  hover.clear();

}

void DrawingPicker::compute_pre_select(float x, float y, PickViewInfo& info)
{
  clear_selection_buffer();
  if (!active() || !ownsPickMode())
  {
    clearHover();
    return;
  }
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    clearHover();
    return;
  }
  try
  {
    const QScopedValueRollback<bool> guard(internalUpdate, true);
    for (const auto& view : factory->views())
    {
      if (view == nullptr || !matchesView(*view, info))
      {
        continue;
      }
      DrawingPickHit hit;
      if (!DrawingHitTest().pick(*view, x, y, snapshot, hit))
      {
        clearHover();
        return;
      }
      auto* entity = new PickedDrawEntity(hit, snapshot.revision);
      const std::array<float, 3> point = {{static_cast<float>(hit.world[0]), static_cast<float>(hit.world[1]), static_cast<float>(hit.world[2])}};
      add_to_selection_buffer(entity, point, static_cast<float>(hit.display[2]), true);
      entity->Delete();
      return;
    }
    clearHover();
  }
  catch (const std::exception& error)
  {
    clearHover();
    PRINT_ERROR("cubit-sim symbol hover failed: %s\n", error.what());
  }
}

void DrawingPicker::compute_pick_by_bounds(PickBounds*, PickViewInfo&)
{
  clear_selection_buffer();
}

void DrawingPicker::notify_pre_select(GPicked::PickedEntity const* entity, const float[3], bool addOrRemove)
{
  if (internalUpdate)
  {
    return;
  }
  const auto* picked = dynamic_cast<const PickedDrawEntity*>(entity);
  if (!active() || !ownsPickMode())
  {
    clearHover();
    return;
  }
  if (!addOrRemove)
  {
    if (picked != nullptr && picked->revision == hoverRevision && picked->handle() == hoverHandle)
    {
      clearHover();
    }
    return;
  }
  if (picked == nullptr || picked->revision != snapshot.revision || picked->handle() == 0 || picked->handle() > snapshot.entities.size())
  {
    clearHover();
    return;
  }
  if (hoverHandle == picked->handle() && hoverRevision == picked->revision)
  {
    return;
  }
  try
  {
    hover.show(snapshot.entities[picked->handle() - 1].geometry, hoverColor);
    hoverHandle = picked->handle();
    hoverRevision = picked->revision;
  }
  catch (const std::exception& error)
  {
    clearHover();
    PRINT_ERROR("cubit-sim hover drawing failed: %s\n", error.what());
  }
}

void DrawingPicker::notify_picked(int num, GPicked::PickedEntity const* const* entities, float const* const*, bool)
{
  if (internalUpdate || !active() || !ownsPickMode() || !dispatch->callback || QCoreApplication::instance() == nullptr)
  {
    return;
  }
  DrawingSelection result;
  for (int index = 0; index < num; ++index)
  {
    const auto* entity = dynamic_cast<const PickedDrawEntity*>(entities[index]);
    if (entity != nullptr && entity->revision == snapshot.revision)
    {
      result = entity->owner;
      break;
    }
  }
  dispatchSelection(result);
}

void DrawingPicker::dispatchSelection(const DrawingSelection& result)
{
  if (!dispatch->callback || QCoreApplication::instance() == nullptr)
  {
    return;
  }
  const auto revision = dispatch->revision;
  const auto serial = ++dispatch->serial;
  const std::weak_ptr<DispatchState> pending = dispatch;
  QTimer::singleShot(0, QCoreApplication::instance(), [pending, revision, serial, result]()
  {
    const auto state = pending.lock();
    if (state && state->active && state->revision == revision && state->serial == serial && state->callback)
    {
      state->callback(result);
    }
  });
}

void DrawingPicker::setHoverColor(const std::string& color)
{
  if (hoverColor != color)
  {
    clearHover();
    hoverColor = color;
  }
}

bool DrawingPicker::pickSymbol(int viewId, double xFraction, double yFraction, DrawingSelection& selection, std::string& error)
{
  selection = {};
  error.clear();
  if (!active() || !ownsPickMode())
  {
    return true;
  }
  if (!std::isfinite(xFraction) || !std::isfinite(yFraction) || xFraction < 0 || xFraction >= 1 || yFraction < 0 || yFraction >= 1)
  {
    error = "Symbol pick coordinates must be inside the render widget.";
    return false;
  }
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    return true;
  }
  try
  {
    std::shared_ptr<GView> view;
    if (viewId >= 0)
    {
      view = factory->find_view_by_id(viewId);
    }
    else if (factory->views().size() == 1)
    {
      // The Qt host has no public render-widget-to-view mapping in this SDK.
      // An unspecified view is safe only when a single registered view exists.
      view = *factory->views().begin();
    }
    if (view == nullptr)
    {
      return true;
    }
    int width = 0;
    int height = 0;
    view->get_size(width, height);
    if (width <= 0 || height <= 0)
    {
      return true;
    }
    const float x = static_cast<float>(xFraction * (width - 1));
    const float y = static_cast<float>((1.0 - yFraction) * (height - 1));
    DrawingPickHit hit;
    if (DrawingHitTest().pick(*view, x, y, snapshot, hit))
    {
      selection = hit.owner;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("cubit-sim symbol hit test failed: ") + exception.what();
    return false;
  }
}
