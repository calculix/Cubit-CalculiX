#include "CalculiXPhysicalModelUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <utility>

namespace {
std::string number_text(double value) {
  std::ostringstream output;
  output.imbue(std::locale::classic());
  output << std::setprecision(17) << value;
  return output.str();
}
bool number_value(const std::string& value, double& output, std::string& error) {
  std::istringstream input(value);
  input.imbue(std::locale::classic());
  input >> output;
  if (!input || !std::isfinite(output)) {
    error = "A finite physical value is required.";
    return false;
  }
  input >> std::ws;
  if (!input.eof()) {
    error = "A numeric physical value is required; select its input unit in workspace units.";
    return false;
  }
  return true;
}
bool scalar(double& value, const std::string& quantity, CalculiXUnitView view, std::string& error) {
  SimUnits::QuantityUnit definition;
  if (!framework_units().quantity(quantity, definition, error)) {
    return false;
  }
  double converted = 0.0;
  if (view == CalculiXUnitView::Solver) {
    if (!framework_units().to_solver(value, definition.dimension, "calculix", converted, error)) {
      return false;
    }
  } else {
    std::string expression;
    std::string workspace = "calculix";
    if (view == CalculiXUnitView::Report) {
      workspace = "report";
    }
    if (!framework_units().display_unit(workspace, quantity, expression, error) || !framework_units().from_si(value, expression, definition.dimension, definition.role, converted, error)) {
      return false;
    }
  }
  value = converted;
  return true;
}
bool scalar(std::string& value, const std::string& quantity, CalculiXUnitView view, std::string& error) {
  if (value.empty()) {
    return true;
  }
  double numeric = 0.0;
  if (!number_value(value, numeric, error) || !scalar(numeric, quantity, view, error)) {
    return false;
  }
  value = number_text(numeric);
  return true;
}
template<class Value>
bool table(std::vector<std::vector<Value>>& data, const std::vector<std::string>& quantities, CalculiXUnitView view, std::string& error) {
  for (auto& row : data) {
    if (row.size() < quantities.size() + 1) {
      error = "Incomplete physical-value row.";
      return false;
    }
    for (std::size_t index = 0; index < quantities.size(); ++index) {
      if (!scalar(row[index + 1], quantities[index], view, error)) {
        return false;
      }
    }
  }
  return true;
}
bool input_number(double& value, const std::string& quantity, std::string& error) {
  SimUnits::QuantityUnit definition;
  std::string expression;
  double converted = 0.0;
  if (!framework_units().quantity(quantity, definition, error) || !framework_units().display_unit("calculix", quantity, expression, error) || !framework_units().to_si(value, expression, definition.dimension, definition.role, converted, error)) {
    return false;
  }
  value = converted;
  return true;
}
bool input_number(std::string& value, const std::string& quantity, std::string& error) {
  if (value.empty()) {
    return true;
  }
  double numeric = 0.0;
  if (!number_value(value, numeric, error) || !input_number(numeric, quantity, error)) {
    return false;
  }
  value = number_text(numeric);
  return true;
}
bool selected(const std::vector<int>& markers, std::size_t index) {
  if (markers.empty()) {
    return true;
  }
  return index < markers.size() && markers[index] == 1;
}
template<class Model>
bool prepare(const Model& model, std::string& error) {
  error.clear();
  if (model.unit_view || model.value_unit_system != "SI") {
    error = "A detached view requires canonical SI physical model values.";
    return false;
  }
  return true;
}
}

bool CalculiXPhysicalModelUnits::input(const std::string& domain, std::vector<std::string>& options, std::vector<double>& values, const std::vector<int>& markers, std::string& error) {
  error.clear();
  std::vector<std::pair<std::size_t, std::string>> fields = {{2, "time"}};
  std::vector<std::string> components;
  if (domain == "forces") {
    components = {"force", "force", "force", "moment", "moment", "moment"};
  } else if (domain == "pressures") {
    components = {"pressure"};
  } else if (domain == "heatfluxes") {
    components = {"heat_flux"};
  } else if (domain == "gravity") {
    fields.emplace_back(7, "acceleration");
  } else if (domain == "centrifugal") {
    fields.emplace_back(7, "frequency_squared");
    fields.emplace_back(8, "length");
    fields.emplace_back(9, "length");
    fields.emplace_back(10, "length");
  } else if (domain == "film") {
    fields.emplace_back(4, "temperature");
    fields.emplace_back(5, "heat_transfer");
    fields.emplace_back(7, "time");
  } else if (domain == "radiation") {
    fields.emplace_back(4, "temperature");
    fields.emplace_back(7, "time");
  } else if (domain == "traction") {
    components = {"force", "force", "force"};
  } else if (domain == "bodyheatflux") {
    fields.emplace_back(4, "body_heat_flux");
  } else if (domain == "displacements") {
    components = {"length", "length", "length", "angle", "angle", "angle"};
  } else if (domain == "temperatures") {
    components = {"temperature"};
  } else {
    error = "Unknown CalculiX physical-value domain.";
    return false;
  }
  for (const auto& field : fields) {
    std::size_t marker = field.first;
    if (domain == "bodyheatflux" && field.first == 4) {
      marker = 5;
    }
    if (!selected(markers, marker)) {
      continue;
    }
    if (field.first >= options.size() || !input_number(options[field.first], field.second, error)) {
      return false;
    }
  }
  if (values.size() < components.size()) {
    error = "Incomplete physical component input.";
    return false;
  }
  for (std::size_t index = 0; index < components.size(); ++index) {
    if (selected(markers, index + 4) && !input_number(values[index], components[index], error)) {
      return false;
    }
  }
  return true;
}
bool CalculiXPhysicalModelUnits::read_basis(HDF5Tool& file, const std::string& group, std::string& basis, std::string& error) {
  basis.clear();
  if (!file.nameExists(group + "/value_units")) {
    return true;
  }
  std::vector<std::string> values;
  if (!file.read_dataset_string_rank_1("value_units", group, values) || values.size() != 1 || (!values.front().empty() && values.front() != "SI")) {
    error = "Invalid physical-value unit metadata.";
    return false;
  }
  basis = values.front();
  return true;
}
bool CalculiXPhysicalModelUnits::write_basis(HDF5Tool& file, const std::string& group, const std::string& basis, std::string& error) {
  if ((!basis.empty() && basis != "SI") || !file.ensureGroup(group) || !file.write_dataset_string_rank_1("value_units", group, {basis})) {
    error = "Cannot save physical-value units: " + file.last_error();
    return false;
  }
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsForces& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.force_data, {"force", "force", "force", "moment", "moment", "moment"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsPressures& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.magnitude_data, {"pressure"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsHeatfluxes& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.magnitude_data, {"heat_flux"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsGravity& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.magnitude_data, {"acceleration"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsCentrifugal& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.magnitude_data, {"frequency_squared"}, view, error)) {
    return false;
  }
  if (!table(candidate.coordinate_data, {"length", "length", "length"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsFilm& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.temperature_data, {"temperature"}, view, error)) {
    return false;
  }
  if (!table(candidate.coefficient_data, {"heat_transfer"}, view, error)) {
    return false;
  }
  if (!table(candidate.film_time_delay_data, {"time"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsRadiation& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.temperature_data, {"temperature"}, view, error)) {
    return false;
  }
  if (!table(candidate.radiation_time_delay_data, {"time"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsSurfaceTraction& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.force_data, {"force", "force", "force"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreLoadsBodyHeatflux& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.magnitude_data, {"body_heat_flux"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreBCsDisplacements& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.displacement_data, {"length", "length", "length", "angle", "angle", "angle"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

bool CalculiXPhysicalModelUnits::convert(CoreBCsTemperatures& model, CalculiXUnitView view, std::string& error) {
  if (!prepare(model, error)) {
    return false;
  }
  auto candidate = model;
  if (!table(candidate.time_delay_data, {"time"}, view, error)) {
    return false;
  }
  if (!table(candidate.magnitude_data, {"temperature"}, view, error)) {
    return false;
  }
  candidate.unit_view = true;
  model = std::move(candidate);
  return true;
}

