#include "FrameworkUnitsInterface.hpp"

FrameworkUnitsInterface::FrameworkUnitsInterface() = default;
SimUnits::CoreFrameworkUnits& FrameworkUnitsInterface::core() const noexcept {
  return framework_units();
}
