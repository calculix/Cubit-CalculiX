#include "UnitSystem.hpp"
#include <cmath>
#include <utility>

SimUnits::UnitSystem::UnitSystem() = default;
SimUnits::UnitSystem::UnitSystem(std::string id, std::string lengthUnit, std::string forceUnit, std::string timeUnit, double lengthScale, double forceScale, double timeScale) : id(std::move(id)), length_unit(std::move(lengthUnit)), force_unit(std::move(forceUnit)), time_unit(std::move(timeUnit)) {
  base_scale[0] = forceScale * timeScale * timeScale / lengthScale;
  base_scale[1] = lengthScale;
  base_scale[2] = timeScale;
}
bool SimUnits::UnitSystem::scale_to_si(const Dimension& dimension, double& output, std::string& error) const {
  error.clear();
  double value = 1.0;
  for (std::size_t index = 0; index < base_scale.size(); ++index) {
    if (!std::isfinite(dimension[index]) || !std::isfinite(base_scale[index]) || base_scale[index] <= 0.0) {
      error = "Invalid physical dimension or unit-system scale.";
      return false;
    }
    value *= std::pow(base_scale[index], dimension[index]);
    if (!std::isfinite(value) || value <= 0.0) {
      error = "Unit-system scale exceeds the supported numeric range.";
      return false;
    }
  }
  output = value;
  return true;
}
