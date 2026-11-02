#include "CoreDrawPicker.hpp"
#include "CoreDrawHitTest.hpp"
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

namespace
{
const char SymbolPickType[] = "ccx_symbol";

const char* featureName(DrawFeature feature)
{
  switch (feature)
  {
    case DrawFeature::Force: return "Force";
    case DrawFeature::Pressure: return "Pressure";
    case DrawFeature::Heatflux: return "Heatflux";
    case DrawFeature::Gravity: return "Gravity";
    case DrawFeature::Centrifugal: return "Centrifugal load";
    case DrawFeature::Trajectory: return "Trajectory";
    case DrawFeature::Film: return "Film";
    case DrawFeature::Radiation: return "Radiation";
    case DrawFeature::SurfaceTraction: return "Surface traction";
    case DrawFeature::BodyHeatflux: return "Body heatflux";
    case DrawFeature::Displacement: return "Displacement";
    case DrawFeature::Temperature: return "Temperature";
    case DrawFeature::Orientation: return "Orientation";
    case DrawFeature::Equation: return "Equation";
    case DrawFeature::EquationGroup: return "Equation group";
    case DrawFeature::Topology: return "Topology";
  }
  return "CalculiX feature";
}

class PickedDrawEntity : public GPicked::PickedEntity
{
public:
  PickedDrawEntity(const DrawPickHit& hit, std::size_t revision)
    : owner(hit.owner), revision(revision), pickHandle(hit.handle), caption(std::string(featureName(hit.owner.entity.feature)) + " " + std::to_string(hit.owner.entity.id))
  {
  }
  const char* type() const override { return SymbolPickType; }
  int id() const override { return owner.entity.id; }
  const char* name() const override { return caption.c_str(); }
  std::size_t handle() const override { return pickHandle; }
  CalculiXDrawSelection owner;
  std::size_t revision;
private:
  std::size_t pickHandle;
  std::string caption;
};
}

CoreDrawPicker::CoreDrawPicker() : dispatch(std::make_shared<DispatchState>())
{
}

CoreDrawPicker::~CoreDrawPicker()
{
  shutdown();
}

bool CoreDrawPicker::ownsPickMode() const
{
  return registered && GPickerBase::get_picker() == this;
}

bool CoreDrawPicker::active() const
{
  return dispatch->active;
}

bool CoreDrawPicker::activate(std::string& error)
{
  error.clear();
  if (active() && ownsPickMode())
  {
    return true;
  }
  const char* previous = GPickerBase::get_pick_type();
  if (previous == nullptr || std::strcmp(previous, SymbolPickType) == 0)
  {
    error = "No previous Cubit pick mode is available to restore.";
    return false;
  }
  previousType = previous;
  if (!registered)
  {
    if (!GPickerBase::register_picker(SymbolPickType, this))
    {
      error = "Cubit could not register CalculiX symbol picking.";
      return false;
    }
    GPickerBase::add_pick_observer(this);
    GPickerBase::add_pre_selection_observer(this);
    registered = true;
  }
  GPickerBase::set_pick_type(SymbolPickType, true);
  dispatch->active = ownsPickMode();
  if (!dispatch->active)
  {
    error = "Cubit could not activate CalculiX symbol picking.";
  }
  return dispatch->active;
}

void CoreDrawPicker::clearOwnedSelections()
{
  const QScopedValueRollback<bool> guard(internalUpdate, true);
  if (dynamic_cast<const PickedDrawEntity*>(GPickerBase::get_current_pre_selection()) != nullptr)
  {
    GPickerBase::set_pre_selection(EntitySelection());
  }
  clear_selection_buffer();
  const EntitySelection* picked = nullptr;
  int count = 0;
  GPickerBase::get_picked_list(picked, count);
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
    GPickerBase::remove_from_picked_list(owned.size(), owned.data());
  }
  const EntitySelection* possible = nullptr;
  GPickerBase::get_possible_selections(possible, count);
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
    GPickerBase::remove_from_possible_selections_list(entity);
  }
}

void CoreDrawPicker::deactivate() noexcept
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
      const int count = GPickerBase::get_number_pick_types();
      std::vector<const char*> types(static_cast<std::size_t>(count));
      GPickerBase::get_pick_types(types.data());
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
        GPickerBase::set_pick_type(restore == nullptr ? fallback : restore, true);
      }
    }
  }
  catch (const std::exception& error)
  {
    PRINT_WARNING("CalculiX pick-mode restoration failed: %s\n", error.what());
  }
}

void CoreDrawPicker::shutdown() noexcept
{
  deactivate();
  dispatch->callback = {};
  if (registered)
  {
    GPickerBase::remove_pre_selection_observer(this);
    GPickerBase::remove_pick_observer(this);
    GPickerBase::unregister_picker(SymbolPickType, this);
    registered = false;
  }
  hover.detach();
  snapshot.entities.clear();
}

void CoreDrawPicker::updateSnapshot(DrawPickSnapshot data)
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

void CoreDrawPicker::setCallback(std::function<void(const CalculiXDrawSelection&)> callback)
{
  dispatch->callback = std::move(callback);
  ++dispatch->serial;
}

void CoreDrawPicker::compute_pick(vtkRenderer* renderer, float x, float y)
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
      if (view != nullptr && view->get_renderer() == renderer)
      {
        DrawPickHit hit;
        CalculiXDrawSelection result;
        if (CoreDrawHitTest().pick(*view, x, y, snapshot, hit))
        {
          auto* entity = new PickedDrawEntity(hit, snapshot.revision);
          const float point[] = {static_cast<float>(hit.world[0]), static_cast<float>(hit.world[1]), static_cast<float>(hit.world[2])};
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
    PRINT_ERROR("CalculiX symbol picking failed: %s\n", error.what());
  }
}

void CoreDrawPicker::clearHover() noexcept
{
  hoverHandle = 0;
  hoverRevision = 0;
  hover.clear();

}

void CoreDrawPicker::compute_pre_select(vtkRenderer* renderer, float x, float y)
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
      if (view == nullptr || view->get_renderer() != renderer)
      {
        continue;
      }
      DrawPickHit hit;
      if (!CoreDrawHitTest().pick(*view, x, y, snapshot, hit))
      {
        clearHover();
        return;
      }
      auto* entity = new PickedDrawEntity(hit, snapshot.revision);
      const float point[] = {static_cast<float>(hit.world[0]), static_cast<float>(hit.world[1]), static_cast<float>(hit.world[2])};
      add_to_selection_buffer(entity, point, static_cast<float>(hit.display[2]), true);
      entity->Delete();
      return;
    }
    clearHover();
  }
  catch (const std::exception& error)
  {
    clearHover();
    PRINT_ERROR("CalculiX symbol hover failed: %s\n", error.what());
  }
}

void CoreDrawPicker::compute_pick_by_bounds(vtkRenderer*, PickBounds*)
{
  clear_selection_buffer();
}

void CoreDrawPicker::notify_pre_select(GPicked::PickedEntity const* entity, const float[3], bool addOrRemove)
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
    PRINT_ERROR("CalculiX hover drawing failed: %s\n", error.what());
  }
}

void CoreDrawPicker::notify_picked(int num, GPicked::PickedEntity const* const* entities, float const* const*, bool)
{
  if (internalUpdate || !active() || !ownsPickMode() || !dispatch->callback || QCoreApplication::instance() == nullptr)
  {
    return;
  }
  CalculiXDrawSelection result;
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

void CoreDrawPicker::dispatchSelection(const CalculiXDrawSelection& result)
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

void CoreDrawPicker::setHoverColor(const std::string& color)
{
  if (hoverColor != color)
  {
    clearHover();
    hoverColor = color;
  }
}

bool CoreDrawPicker::pickSymbol(int viewId, double xFraction, double yFraction, CalculiXDrawSelection& selection, std::string& error)
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
    DrawPickHit hit;
    if (CoreDrawHitTest().pick(*view, x, y, snapshot, hit))
    {
      selection = hit.owner;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("CalculiX symbol hit test failed: ") + exception.what();
    return false;
  }
}
