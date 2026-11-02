#ifndef COREDRAWPICKDATA_HPP
#define COREDRAWPICKDATA_HPP

#include "CoreDrawTypes.hpp"
#include "CoreDrawGeometry.hpp"
#include <array>
#include <cstddef>

struct DrawPickEntity
{
  CalculiXDrawSelection owner;
  DrawGeometry geometry;
};

struct DrawPickSnapshot
{
  std::size_t revision = 0;
  std::vector<DrawPickEntity> entities;
};

struct DrawPickHit
{
  CalculiXDrawSelection owner;
  std::array<double, 3> world{};
  std::array<double, 3> display{};
  double distanceSquared = 0;
  std::size_t handle = 0;
};

#endif
