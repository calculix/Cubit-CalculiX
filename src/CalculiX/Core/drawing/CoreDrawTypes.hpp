#ifndef COREDRAWTYPES_HPP
#define COREDRAWTYPES_HPP

#include <map>
#include <string>
#include <vector>

enum class DrawGroup { BoundaryConditions, Loads, Constraints, Orientations, Topology };
enum class DrawScopeKind { Model, Step, Custom };
enum class DrawFeature
{
  Force, Pressure, Heatflux, Gravity, Centrifugal, Trajectory, Film, Radiation,
  SurfaceTraction, BodyHeatflux, Displacement, Temperature, Orientation,
  Equation, EquationGroup, Topology
};

struct DrawEntityKey
{
  DrawFeature feature = DrawFeature::Force;
  int id = -1;
  bool operator<(const DrawEntityKey& other) const
  {
    return feature < other.feature || (feature == other.feature && id < other.id);
  }
};

struct CalculiXDrawSelection
{
  bool active = false;
  DrawEntityKey entity;
  int stepId = -1;
};

struct DrawScope
{
  DrawScopeKind kind = DrawScopeKind::Model;
  int stepId = -1;
};

struct DrawGroupState
{
  bool enabled = false;
  bool initialized = false;
  bool dirty = true;
  bool allInScope = true;
  DrawScope sourceScope;
  std::vector<DrawFeature> featureFilter;
  std::vector<DrawEntityKey> entityFilter;
  std::vector<DrawEntityKey> entities;
};

struct CalculiXDrawState
{
  DrawScope scope;
  double sizeFactor = 1.0;
  bool overlayVisible = true;
  bool pickingEnabled = false;
  std::map<DrawGroup, DrawGroupState> groups;
};

DrawGroup drawing_group(DrawFeature feature);
const char* drawing_group_name(DrawGroup group);
#endif
