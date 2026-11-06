#ifndef CUBITSIM_DRAWINGPICKDATA_HPP
#define CUBITSIM_DRAWINGPICKDATA_HPP

#include "DrawingTypes.hpp"
#include "DrawingGeometry.hpp"
#include <array>
#include <cstddef>

struct DrawingPickEntity
{
  DrawingSelection owner;
  DrawingGeometry geometry;
};

struct DrawingPickSnapshot
{
  std::size_t revision = 0;
  std::vector<DrawingPickEntity> entities;
};

struct DrawingPickHit
{
  DrawingSelection owner;
  std::array<double, 3> world{};
  std::array<double, 3> display{};
  double distanceSquared = 0;
  std::size_t handle = 0;
};

#endif
