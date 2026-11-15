#include "CalculiXDomainUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <utility>
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"

namespace {
bool scalar(double& value, const std::string& quantity, CalculiXDomainUnitView view, std::string& error) {
  SimUnits::QuantityUnit definition;
  if (!framework_units().quantity(quantity, definition, error)) {
    return false;
  }
  double converted = 0.0;
  std::string workspace = "calculix";
  if (view == CalculiXDomainUnitView::Report) {
    workspace = "report";
  }
  if (view == CalculiXDomainUnitView::Native) {
    SimUnits::WorkspaceUnitContext context;
    if (!framework_units().workspace_context("calculix", context, error)) {
      return false;
    }
    if (context.native_system.empty()) {
      error = "Saved physical values have no declared source units.";
      return false;
    }
    if (quantity == "angle") {
      if (!framework_units().to_si(value, "deg", definition.dimension, definition.role, converted, error)) {
        return false;
      }
    } else if (!framework_units().system_to_si(value, definition.dimension, context.native_system, converted, error)) {
      return false;
    }
  } else if (view == CalculiXDomainUnitView::Solver && quantity != "angle") {
    if (!framework_units().to_solver(value, definition.dimension, "calculix", converted, error)) {
      return false;
    }
  } else {
    std::string unit;
    if (view == CalculiXDomainUnitView::Geometry && quantity == "length") {
      unit = framework_units().geometry_unit();
    } else if (view == CalculiXDomainUnitView::Solver || (view == CalculiXDomainUnitView::Geometry && quantity == "angle")) {
      unit = "deg";
    } else if (!framework_units().display_unit(workspace, quantity, unit, error)) {
      return false;
    }
    if (view == CalculiXDomainUnitView::Input) {
      if (!framework_units().to_si(value, unit, definition.dimension, definition.role, converted, error)) {
        return false;
      }
    } else if (!framework_units().from_si(value, unit, definition.dimension, definition.role, converted, error)) {
      return false;
    }
  }
  if (!std::isfinite(converted)) {
    error = "A converted model value is not finite.";
    return false;
  }
  value = converted;
  return true;
}
bool scalar(std::string& value, const std::string& quantity, CalculiXDomainUnitView view, std::string& error) {
  if (value.empty()) {
    return true;
  }
  std::istringstream input(value);
  input.imbue(std::locale::classic());
  double numeric = 0.0;
  input >> numeric;
  if (!input || !std::isfinite(numeric)) {
    error = "A finite numeric model value is required: " + value;
    return false;
  }
  input >> std::ws;
  if (!input.eof()) {
    error = "Unexpected text in a numeric model value: " + value;
    return false;
  }
  if (!scalar(numeric, quantity, view, error)) {
    return false;
  }
  std::ostringstream output;
  output.imbue(std::locale::classic());
  output << std::setprecision(17) << numeric;
  value = output.str();
  return true;
}
template<class Value>
bool fields(std::vector<std::vector<Value>>& rows, const std::vector<std::pair<std::size_t, std::string>>& mapping, CalculiXDomainUnitView view, std::string& error) {
  for (auto& row : rows) {
    for (const auto& field : mapping) {
      if (field.first >= row.size()) {
        error = "Incomplete physical model record.";
        return false;
      }
      if (!scalar(row[field.first], field.second, view, error)) {
        return false;
      }
    }
  }
  return true;
}
}

bool CalculiXDomainUnits::convert(CoreSections& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.solid_section_data, {{4, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.shell_section_data, {{4, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.membrane_section_data, {{4, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.beam_section_data, {{4, "length"}, {5, "length"}, {6, "length"}, {7, "length"}, {8, "length"}, {9, "length"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreOrientations& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.a_data, {{1, "length"}, {2, "length"}, {3, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.b_data, {{1, "length"}, {2, "length"}, {3, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.rotation_data, {{1, "angle"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreConstraints& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.tie_constraint_data, {{4, "length"}}, view, error)) {
    return false;
  }
  for (auto& row : candidate.equation_data) {
    if (row.size() != 4 || !std::isfinite(row[2]) || !std::isfinite(row[3]) || std::floor(row[2]) != row[2]) {
      error = "Incomplete constraint equation term.";
      return false;
    }
    std::string quantity;
    if (row[2] >= 1 && row[2] <= 3) {
      quantity = "length";
    } else if (row[2] >= 4 && row[2] <= 6) {
      quantity = "angle";
    } else if (row[2] == 11) {
      quantity = "temperature_difference";
    } else {
      error = "Unsupported equation degree of freedom.";
      return false;
    }
    double scale = 1.0;
    if (view == CalculiXDomainUnitView::Solver && quantity == "angle") {
      scale = 1.0;
    } else if (!scalar(scale, quantity, view, error)) {
      return false;
    }
    if (!std::isfinite(scale) || scale == 0.0 || !std::isfinite(row[3] / scale)) {
      error = "Constraint equation coefficient cannot be represented in the selected units.";
      return false;
    }
    row[3] /= scale;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreContactPairs& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.adjust_contactpair_data, {{1, "length"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreSurfaceInteractions& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.exponential_surfacebehavior_data, {{1, "length"}, {2, "stress"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.linear_surfacebehavior_data, {{1, "force_per_volume"}, {2, "stress"}, {3, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.tabular_surfacebehavior_data, {{1, "stress"}, {2, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.tied_surfacebehavior_data, {{1, "force_per_volume"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.gap_conductance_data, {{1, "heat_transfer"}, {2, "stress"}, {3, "temperature"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.gap_heat_generation_data, {{3, "velocity"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.friction_data, {{2, "force_per_volume"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreSteps& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.static_data, {{5, "time"}, {6, "time"}, {7, "time"}, {8, "time"}, {9, "time"}, {10, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.frequency_data, {{6, "frequency"}, {7, "frequency"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.heattransfer_data, {{7, "temperature_difference"}, {9, "time"}, {10, "time"}, {11, "time"}, {12, "time"}, {13, "time"}, {15, "frequency"}, {16, "frequency"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.coupledtd_data, {{5, "temperature_difference"}, {7, "time"}, {9, "time"}, {10, "time"}, {11, "time"}, {12, "time"}, {13, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.uncoupledtd_data, {{5, "temperature_difference"}, {8, "time"}, {9, "time"}, {10, "time"}, {11, "time"}, {12, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.dynamic_data, {{6, "time"}, {7, "time"}, {8, "time"}, {9, "time"}, {10, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.modal_dynamic_data, {{5, "time"}, {7, "time"}, {8, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.steady_state_dynamics_data, {{3, "frequency"}, {4, "frequency"}, {8, "time"}, {9, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_static_data, {{5, "time"}, {6, "time"}, {7, "time"}, {8, "time"}, {9, "time"}, {10, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_frequency_data, {{6, "frequency"}, {7, "frequency"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_heattransfer_data, {{7, "temperature_difference"}, {9, "time"}, {10, "time"}, {11, "time"}, {12, "time"}, {13, "time"}, {15, "frequency"}, {16, "frequency"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_coupledtd_data, {{5, "temperature_difference"}, {7, "time"}, {9, "time"}, {10, "time"}, {11, "time"}, {12, "time"}, {13, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_uncoupledtd_data, {{5, "temperature_difference"}, {8, "time"}, {9, "time"}, {10, "time"}, {11, "time"}, {12, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_dynamic_data, {{6, "time"}, {7, "time"}, {8, "time"}, {9, "time"}, {10, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_modal_dynamic_data, {{5, "time"}, {7, "time"}, {8, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.backup_steady_state_dynamics_data, {{3, "frequency"}, {4, "frequency"}, {8, "time"}, {9, "time"}}, view, error)) {
    return false;
  }
  for (auto& row : candidate.modal_dynamic_data) {
    if (row.size() < 9) {
      error = "Incomplete modal dynamic procedure.";
      return false;
    }
    if (row[4].empty()) {
      if (!scalar(row[6], "time", view, error)) {
        return false;
      }
    }
    if (!scalar(row[3], "modal_force", view, error)) {
      return false;
    }
  }
  for (auto& row : candidate.backup_modal_dynamic_data) {
    if (row.size() < 9) {
      error = "Incomplete modal dynamic procedure.";
      return false;
    }
    if (row[4].empty()) {
      if (!scalar(row[6], "time", view, error)) {
        return false;
      }
    }
    if (!scalar(row[3], "modal_force", view, error)) {
      return false;
    }
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreInitialConditions& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.stress_element_data, {{3, "stress"}, {4, "stress"}, {5, "stress"}, {6, "stress"}, {7, "stress"}, {8, "stress"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.stress_block_data, {{2, "stress"}, {3, "stress"}, {4, "stress"}, {5, "stress"}, {6, "stress"}, {7, "stress"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreDamping& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (candidate.damping_data.size() > 0 && !scalar(candidate.damping_data[0], "inverse_time", view, error)) {
    return false;
  }
  if (candidate.damping_data.size() > 1 && !scalar(candidate.damping_data[1], "time", view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CorePhysicalConstants& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (candidate.physicalconstants_data.size() > 0 && !scalar(candidate.physicalconstants_data[0], "temperature", view, error)) {
    return false;
  }
  if (candidate.physicalconstants_data.size() > 1 && !scalar(candidate.physicalconstants_data[1], "stefan_boltzmann", view, error)) {
    return false;
  }
  if (candidate.physicalconstants_data.size() > 2 && !scalar(candidate.physicalconstants_data[2], "newton_gravity", view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreAmplitudes& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  for (const auto& amplitude : candidate.amplitudes_data) {
    if (amplitude.size() < 6) {
      error = "Incomplete amplitude record.";
      return false;
    }
    std::string quantity = "time";
    const auto axis = candidate.axis_quantities.find(amplitude[0]);
    if (axis != candidate.axis_quantities.end()) {
      quantity = axis->second;
    }
    if (quantity != "time" && quantity != "frequency") {
      error = "Amplitude axis must be time or frequency.";
      return false;
    }
    for (auto& shift : candidate.shiftx_amplitude_data) {
      if (shift.size() < 2) {
        error = "Incomplete amplitude shift.";
        return false;
      }
      if (shift[0] == std::to_string(amplitude[2]) && !scalar(shift[1], quantity, view, error)) {
        return false;
      }
    }
    for (auto& point : candidate.amplitudevalues_amplitude_data) {
      if (point.size() < 3) {
        error = "Incomplete amplitude point.";
        return false;
      }
      if (point[0] == amplitude[5] && !scalar(point[1], quantity, view, error)) {
        return false;
      }
    }
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreLoadsTrajectoryHeatflux& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.time_data, {{1, "time"}, {2, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.radius_data, {{1, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.magnitude_data, {{1, "heat_flux"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}

bool CalculiXDomainUnits::convert(CoreLoadsTrajectoryBodyHeatfluxSphere& model, CalculiXDomainUnitView view, std::string& error) {
  error.clear();
  auto candidate = model;
  if (!fields(candidate.time_data, {{1, "time"}, {2, "time"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.radius_data, {{1, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.depth_data, {{1, "length"}}, view, error)) {
    return false;
  }
  if (!fields(candidate.magnitude_data, {{1, "body_heat_flux"}}, view, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  if (view == CalculiXDomainUnitView::Input || view == CalculiXDomainUnitView::Native) {
    candidate.domain_unit_view = false;
  }
  model = std::move(candidate);
  return true;
}
