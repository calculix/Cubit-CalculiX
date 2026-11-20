#pragma once
#include <array>
#include <string>

class CoreJointGeometry
{
public:
  static bool coaxial(int surface_a, int surface_b, std::array<double, 3>& center,
    std::array<double, 3>& direction, std::string& error);
};
