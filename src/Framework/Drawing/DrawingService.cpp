#include "DrawingService.hpp"
#include "CubitMessage.hpp"
#include "CubitColorTable.hpp"
#include <QCoreApplication>
#include <QThread>
#include <cmath>
#include <exception>
#include <set>
#include <utility>

namespace
{
bool guiThread(std::string& error)
{
  error.clear();
  auto* app = QCoreApplication::instance();
  if (app == nullptr || QCoreApplication::closingDown() || QThread::currentThread() != app->thread())
  {
    error = "Drawing requires the live Cubit application thread.";
    return false;
  }
  return true;
}

bool matches(const DrawingSelection& selection, const DrawingEntityKey& entity)
{
  return selection.active && selection.entity.workspace == entity.workspace &&
    selection.entity.kind == entity.kind && selection.entity.id == entity.id &&
    (selection.entity.context.empty() || selection.entity.context == entity.context);
}

bool validGeometry(const DrawingGeometry& geometry)
{
  for (const auto& primitive : geometry.primitives)
  {
    int color = -1;
    if (!CubitColorTable::string_to_color_id(primitive.color.c_str(), color) ||
        (primitive.kind != DrawingPrimitiveKind::Point && primitive.kind != DrawingPrimitiveKind::Line &&
         primitive.kind != DrawingPrimitiveKind::Polygon && primitive.kind != DrawingPrimitiveKind::Label))
    {
      return false;
    }
    const auto count = primitive.points.size();
    if ((primitive.kind == DrawingPrimitiveKind::Point && count != 1) ||
        (primitive.kind == DrawingPrimitiveKind::Line && count != 2) ||
        (primitive.kind == DrawingPrimitiveKind::Polygon && count < 3) ||
        (primitive.kind == DrawingPrimitiveKind::Label && count != 1))
    {
      return false;
    }
    for (const auto& point : primitive.points)
    {
      if (point.size() != 3)
      {
        return false;
      }
      for (double value : point)
      {
        if (!std::isfinite(value))
        {
          return false;
        }
      }
    }
  }
  return true;
}
}

DrawingService::DrawingService() = default;

DrawingService::~DrawingService()
{
  picker.shutdown();
  graphics.detach();
}

bool DrawingService::submit(const std::string& workspace, DrawingFrame frame, std::string& error)
{
  if (!guiThread(error))
  {
    return false;
  }
  if (workspace.empty() || frame.style.hoverColor.empty() || frame.style.selectionColor.empty() ||
      (frame.selection.active && frame.selection.entity.workspace != workspace))
  {
    error = "Drawing frame has an invalid workspace, selection owner or missing style.";
    return false;
  }
  int color = -1;
  if (!CubitColorTable::string_to_color_id(frame.style.hoverColor.c_str(), color) ||
      !CubitColorTable::string_to_color_id(frame.style.selectionColor.c_str(), color))
  {
    error = "Drawing frame contains an unknown hover or selection color.";
    return false;
  }
  std::set<DrawingEntityKey> seen;
  for (const auto& entity : frame.entities)
  {
    if (entity.key.workspace != workspace || entity.key.kind.empty() || entity.key.id < 0 ||
        entity.layer.empty() || frame.layers.count(entity.layer) == 0 ||
        !seen.insert(entity.key).second || !validGeometry(entity.geometry))
    {
      error = "Drawing frame contains invalid, duplicate or foreign entities.";
      return false;
    }
  }
  frames[workspace] = std::move(frame);
  return workspace != activeWorkspace || render(error);
}

void DrawingService::set_pick_callback(const std::string& workspace, DrawingPickCallback callback)
{
  callbacks[workspace] = std::move(callback);
  if (workspace == activeWorkspace)
  {
    // Replacing a callback invalidates any queued invocation of the old owner.
    const auto current = callbacks.find(workspace);
    picker.setCallback(current == callbacks.end() ? DrawingPickCallback{} : current->second);
  }
}

bool DrawingService::activate_workspace(const std::string& workspace, std::string& error)
{
  if (!guiThread(error))
  {
    return false;
  }
  if (workspace.empty())
  {
    error = "The active drawing workspace must have an identifier.";
    return false;
  }
  if (workspace == activeWorkspace)
  {
    return true;
  }
  const auto previous = activeWorkspace;
  picker.deactivate();
  picker.setCallback({});
  graphics.detach();
  activeWorkspace = workspace;
  if (render(error))
  {
    return true;
  }
  const auto failure = error;
  picker.deactivate();
  graphics.detach();
  activeWorkspace = previous;
  std::string rollback;
  render(rollback);
  error = failure;
  return false;
}

bool DrawingService::render(std::string& error)
{
  error.clear();
  const auto found = frames.find(activeWorkspace);
  if (found == frames.end())
  {
    picker.deactivate();
    picker.setCallback({});
    picker.updateSnapshot({});
    graphics.detach();
    return true;
  }
  auto& frame = found->second;
  const auto callback = callbacks.find(activeWorkspace);
  picker.setCallback(callback == callbacks.end() ? DrawingPickCallback{} : callback->second);
  DrawingPickSnapshot snapshot;
  snapshot.revision = ++revision;
  std::map<std::string, DrawingGeometry> groups;
  for (const auto& entity : frame.entities)
  {
    const auto layer = frame.layers.find(entity.layer);
    if (layer == frame.layers.end() || !layer->second.enabled || !frame.overlayVisible)
    {
      continue;
    }
    if (entity.pickable && frame.pickingEnabled)
    {
      snapshot.entities.push_back({DrawingSelection{true, entity.key}, entity.geometry});
    }
    auto geometry = entity.geometry;
    if (matches(frame.selection, entity.key))
    {
      for (auto& primitive : geometry.primitives)
      {
        primitive.color = frame.style.selectionColor;
      }
    }
    groups[entity.layer].append(geometry);
  }
  picker.updateSnapshot(std::move(snapshot));
  if (picker.active() && !picker.ownsPickMode())
  {
    // Respect native pick modes explicitly chosen after symbol picking.
    frame.pickingEnabled = false;
  }
  if (!frame.pickingEnabled || !frame.overlayVisible)
  {
    picker.deactivate();
  }
  picker.setHoverColor(frame.style.hoverColor);
  if (!graphics.synchronize(activeWorkspace, groups, frame, error))
  {
    picker.deactivate();
    return false;
  }
  if (frame.pickingEnabled && frame.overlayVisible && !picker.activate(error))
  {
    frame.pickingEnabled = false;
    return false;
  }
  return true;
}

bool DrawingService::picking_active(const std::string& workspace) const
{
  const auto frame = frames.find(workspace);
  return workspace == activeWorkspace && frame != frames.end() && frame->second.overlayVisible &&
    frame->second.pickingEnabled && picker.active() && picker.ownsPickMode();
}

bool DrawingService::pick(const std::string& workspace, int viewId, double x, double y, DrawingSelection& selection, std::string& error)
{
  selection = {};
  if (!guiThread(error))
  {
    return false;
  }
  if (!picking_active(workspace))
  {
    return true;
  }
  return picker.pickSymbol(viewId, x, y, selection, error);
}

void DrawingService::clear_hover(const std::string& workspace) noexcept
{
  if (workspace == activeWorkspace)
  {
    picker.clearHover();
  }
}

void DrawingService::clear(const std::string& workspace) noexcept
{
  frames.erase(workspace);
  callbacks.erase(workspace);
  if (workspace == activeWorkspace)
  {
    picker.shutdown();
    graphics.detach();
  }
}
