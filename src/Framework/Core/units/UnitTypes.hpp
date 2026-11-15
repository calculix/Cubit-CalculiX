#pragma once
#include <array>

namespace SimUnits {
using Dimension = std::array<double, 7>;
enum class UnitRole { Linear, AbsoluteTemperature, TemperatureDifference, Angle };
inline constexpr Dimension Dimensionless = {0, 0, 0, 0, 0, 0, 0};
inline constexpr Dimension Mass = {1, 0, 0, 0, 0, 0, 0};
inline constexpr Dimension Length = {0, 1, 0, 0, 0, 0, 0};
inline constexpr Dimension Time = {0, 0, 1, 0, 0, 0, 0};
inline constexpr Dimension Temperature = {0, 0, 0, 1, 0, 0, 0};
inline constexpr Dimension Force = {1, 1, -2, 0, 0, 0, 0};
inline constexpr Dimension Stress = {1, -1, -2, 0, 0, 0, 0};
inline constexpr Dimension Density = {1, -3, 0, 0, 0, 0, 0};
inline constexpr Dimension Energy = {1, 2, -2, 0, 0, 0, 0};
inline constexpr Dimension Power = {1, 2, -3, 0, 0, 0, 0};
inline constexpr Dimension SpecificHeat = {0, 2, -2, -1, 0, 0, 0};
inline constexpr Dimension Conductivity = {1, 1, -3, -1, 0, 0, 0};
inline constexpr Dimension Expansion = {0, 0, 0, -1, 0, 0, 0};
inline constexpr Dimension InverseStress = {-1, 1, 2, 0, 0, 0, 0};
inline constexpr Dimension InverseTime = {0, 0, -1, 0, 0, 0, 0};
}
