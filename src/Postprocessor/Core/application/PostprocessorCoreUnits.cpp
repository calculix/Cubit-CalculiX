#include "PostprocessorCore.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <limits>

bool Postprocessor::PostprocessorCore::configure_units(const std::string& system, const std::string& quantity, const std::string& unit, std::string& error) {
  if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() || state.plot_revision == std::numeric_limits<std::uint64_t>::max()) {
    error = "Cannot change units during a source operation or after runtime identifiers are exhausted.";
    return false;
  }
  SimUnits::WorkspaceUnitContext previous;
  if (!framework_units().workspace_context("postprocessor", previous, error)) {
    return false;
  }
  auto candidate = state;
  std::map<std::string, std::string> oldUnits;
  for (const auto& entry : state.representations) {
    const auto& display = entry.second;
    const auto source = result_data(output_source(display.input));
    if (!source || !display.coloring.field || display.increment_index >= source->increments.size()) {
      continue;
    }
    const auto& selected = *display.coloring.field;
    const auto* field = find_field(source->increments[display.increment_index], selected);
    if (!field) {
      continue;
    }
    SimUnits::QuantityUnit descriptor;
    if (!SimResults::ResultsUnits::field_quantity(*field, selected.component, selected.magnitude, descriptor, error)) {
      return false;
    }
    if (!descriptor.unit.empty() && !descriptor.quantity.empty()) {
      if (!framework_units().display_unit("postprocessor", descriptor.quantity, oldUnits[entry.first], error)) {
        return false;
      }
    }
  }
  bool changed = false;
  if (!system.empty()) {
    changed = framework_units().set_workspace_system("postprocessor", system, error);
  } else {
    changed = framework_units().set_display_unit("postprocessor", quantity, unit, error);
  }
  if (!changed) {
    return false;
  }
  const auto rollback = [&]() {
    std::string ignored;
    framework_units().set_workspace_context(previous, ignored);
  };
  for (auto& entry : candidate.representations) {
    const auto prior = oldUnits.find(entry.first);
    if (prior == oldUnits.end()) {
      continue;
    }
    auto& display = entry.second;
    const auto source = result_data(output_source(display.input));
    const auto& selected = *display.coloring.field;
    const auto* field = find_field(source->increments[display.increment_index], selected);
    double converted = 0.0;
    std::string target;
    if (!SimResults::ResultsUnits::field_value(0.0, *field, selected.component, selected.magnitude, "postprocessor", converted, target, error)) {
      rollback();
      return false;
    }
    for (double* value : {&display.coloring.fixed_range[0], &display.coloring.fixed_range[1], &display.coloring.minimum.value, &display.coloring.maximum.value}) {
      if (!framework_units().convert(*value, prior->second, target, converted, error)) {
        rollback();
        return false;
      }
      *value = converted;
    }
  }
  ++candidate.plot_revision;
  for (auto& entry : candidate.plots) {
    entry.second.axes.x.reset();
    entry.second.axes.y.reset();
    entry.second.data_revision = candidate.plot_revision;
  }
  state.swap(candidate);
  for (auto& entry : runtime) {
    entry.second.all_frame_ranges.clear();
  }
  ++scene_revision;
  return true;
}
