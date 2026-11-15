#include "ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
#include <cmath>

bool SimResults::ResultsUnits::display_value(double input, const SimUnits::QuantityUnit& source, const std::string& workspace, double& output, std::string& label, std::string& error) {
  output = input;
  label.clear();
  error.clear();
  if (source.unit.empty()) {
    label = "unknown unit";
    return true;
  }
  label = source.unit;
  if (source.quantity.empty()) {
    return true;
  }
  if (!framework_units().display_unit(workspace, source.quantity, label, error)) {
    return false;
  }
  if (!std::isfinite(input)) {
    return true;
  }
  double canonical = 0.0;
  if (!framework_units().to_si(input, source.unit, source.dimension, source.role, canonical, error)) {
    return false;
  }
  return framework_units().from_si(canonical, label, source.dimension, source.role, output, error);
}

bool SimResults::ResultsUnits::field_quantity(const Field& field, const std::string& component, bool magnitude, SimUnits::QuantityUnit& output, std::string& error) {
  output = SimUnits::QuantityUnit();
  error.clear();
  if (field.categorical) {
    error = "Categorical material or entity IDs cannot be used as physical result values.";
    return false;
  }
  if (field.component_units.empty()) {
    return true;
  }
  if (field.component_units.size() != field.components.size()) {
    error = "Result quantity metadata does not match the component count.";
    return false;
  }
  if (magnitude) {
    if (!field.vector_components) {
      error = "The result has no physical vector components.";
      return false;
    }
    const auto first = (*field.vector_components)[0];
    if (first >= field.component_units.size()) {
      error = "Invalid vector quantity component.";
      return false;
    }
    output = field.component_units[first];
    for (auto index : *field.vector_components) {
      if (index >= field.component_units.size()) {
        error = "Invalid vector quantity component.";
        return false;
      }
      const auto& quantity = field.component_units[index];
      if (quantity.unit != output.unit || quantity.dimension != output.dimension || quantity.role != output.role || quantity.role == SimUnits::UnitRole::AbsoluteTemperature) {
        error = "Vector magnitude requires compatible linear component units.";
        return false;
      }
    }
    return true;
  }
  const auto found = std::find(field.components.begin(), field.components.end(), component);
  if (found == field.components.end()) {
    error = "The selected result component is unavailable.";
    return false;
  }
  output = field.component_units[static_cast<std::size_t>(found - field.components.begin())];
  return true;
}

bool SimResults::ResultsUnits::field_value(double input, const Field& field, const std::string& component, bool magnitude, const std::string& workspace, double& output, std::string& label, std::string& error) {
  SimUnits::QuantityUnit quantity;
  if (!field_quantity(field, component, magnitude, quantity, error)) {
    return false;
  }
  return display_value(input, quantity, workspace, output, label, error);
}
