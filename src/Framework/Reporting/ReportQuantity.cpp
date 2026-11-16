#include "ReportQuantity.hpp"
#include "Framework/Core/units/QuantityUnit.hpp"

ReportQuantity::ReportQuantity() = default;

ReportQuantity::ReportQuantity(const SimUnits::QuantityUnit& value)
  : quantity(value.quantity), unit(value.unit), dimension(value.dimension), role(value.role)
{
}
