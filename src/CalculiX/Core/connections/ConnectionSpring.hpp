#pragma once
#include "ConnectionEndpoint.hpp"
#include <optional>

struct ConnectionSpring
{
  int id = 0;
  std::string name;
  ConnectionEndpoint first;
  std::optional<ConnectionEndpoint> second;
  int direction = 0; // Axial: 0. Ground: global X, Y or Z as 1, 2 or 3.
  double stiffness_si = 0.0;
  int history_step = -1;
  bool validate(std::string& error) const;
  bool uses_point(int point) const;
};
