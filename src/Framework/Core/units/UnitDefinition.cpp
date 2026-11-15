#include "UnitDefinition.hpp"
#include <utility>

SimUnits::UnitDefinition::UnitDefinition() = default;
SimUnits::UnitDefinition::UnitDefinition(std::string id, Dimension dimension, double scale, UnitRole role, double offset) : id(std::move(id)), dimension(dimension), scale(scale), role(role), offset(offset) {
}
