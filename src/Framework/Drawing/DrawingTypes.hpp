#ifndef CUBITSIM_DRAWINGTYPES_HPP
#define CUBITSIM_DRAWINGTYPES_HPP

#include "DrawingGeometry.hpp"
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

struct DrawingEntityKey
{
  std::string workspace;
  std::string kind;
  int id = -1;
  // Opaque workspace-defined scope. Empty selection context matches any context.
  std::string context;
  bool operator<(const DrawingEntityKey& other) const
  {
    return std::tie(workspace, kind, id, context) < std::tie(other.workspace, other.kind, other.id, other.context);
  }
};

struct DrawingSelection
{
  bool active = false;
  DrawingEntityKey entity;
};

struct DrawingEntity
{
  DrawingEntityKey key;
  std::string layer;
  DrawingGeometry geometry;
  bool pickable = true;
};

struct DrawingLayerState
{
  bool enabled = false;
  // An enabled, nonempty layer may temporarily replace the native model view.
  bool hideNativeModel = false;
};

struct DrawingStyle
{
  // Required from the owning workspace Core; the renderer supplies no defaults.
  std::string hoverColor;
  std::string selectionColor;
};

struct DrawingFrame
{
  std::map<std::string, DrawingLayerState> layers;
  std::vector<DrawingEntity> entities;
  DrawingSelection selection;
  DrawingStyle style;
  bool overlayVisible = true;
  bool pickingEnabled = false;
};

using DrawingPickCallback = std::function<void(const DrawingSelection&)>;
#endif
