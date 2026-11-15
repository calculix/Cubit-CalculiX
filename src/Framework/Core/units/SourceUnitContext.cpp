#include "SourceUnitContext.hpp"

SimUnits::SourceUnitContext::SourceUnitContext() = default;
bool SimUnits::SourceUnitContext::known() const noexcept {
  return !system.empty();
}
