#ifndef COREDRAWSAVEDSTATE_HPP
#define COREDRAWSAVEDSTATE_HPP

#include "CoreDrawTypes.hpp"

struct DrawSavedGroup
{
  DrawGroup group = DrawGroup::BoundaryConditions;
  bool enabled = false;
  bool initialized = false;
  bool allInScope = true;
  DrawScope sourceScope;
  std::vector<DrawFeature> featureFilter;
  std::vector<DrawEntityKey> entityFilter;
  std::vector<DrawEntityKey> entities;
};

struct DrawSavedState
{
  DrawScope scope;
  double sizeFactor = 1.0;
  bool overlayVisible = true;
  std::vector<DrawSavedGroup> groups;
};

#endif
