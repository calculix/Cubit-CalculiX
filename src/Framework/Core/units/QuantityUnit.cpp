#include "QuantityUnit.hpp"
#include <utility>

SimUnits::QuantityUnit::QuantityUnit() = default;
SimUnits::QuantityUnit::QuantityUnit(std::string quantity, std::string unit, Dimension dimension, UnitRole role) : quantity(std::move(quantity)), unit(std::move(unit)), dimension(dimension), role(role) {
}
