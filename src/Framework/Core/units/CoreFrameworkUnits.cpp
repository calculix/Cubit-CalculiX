#include "CoreFrameworkUnits.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cctype>
#include <functional>
#include <iomanip>
#include <locale>
#include <sstream>
#include <utility>

namespace {
using namespace SimUnits;

void skip_space(const std::string& text, std::size_t& position) {
  while (position < text.size() && std::isspace(static_cast<unsigned char>(text[position]))) {
    ++position;
  }
}

bool parse_product(const std::string& text, std::size_t& position, const std::map<std::string, UnitDefinition>& definitions, UnitDefinition& output, std::string& error, int depth);

bool parse_factor(const std::string& text, std::size_t& position, const std::map<std::string, UnitDefinition>& definitions, UnitDefinition& output, std::string& error, int depth) {
  skip_space(text, position);
  if (depth > 16 || position >= text.size()) {
    error = "Incomplete or excessively nested unit expression.";
    return false;
  }
  if (text[position] == '(') {
    ++position;
    if (!parse_product(text, position, definitions, output, error, depth + 1)) {
      return false;
    }
    skip_space(text, position);
    if (position >= text.size() || text[position] != ')') {
      error = "Unclosed unit-expression parentheses.";
      return false;
    }
    ++position;
  } else {
    const auto first = position;
    while (position < text.size()) {
      const char value = text[position];
      if (value == '*' || value == '/' || value == '^' || value == '(' || value == ')' || std::isspace(static_cast<unsigned char>(value))) {
        break;
      }
      ++position;
    }
    const auto found = definitions.find(text.substr(first, position - first));
    if (found == definitions.end()) {
      error = "Unknown unit symbol: " + text.substr(first, position - first);
      return false;
    }
    output = found->second;
  }
  skip_space(text, position);
  if (position < text.size() && text[position] == '^') {
    ++position;
    skip_space(text, position);
    double exponent = 0.0;
    const auto parsed = std::from_chars(text.data() + position, text.data() + text.size(), exponent);
    if (parsed.ec != std::errc{} || parsed.ptr == text.data() + position || !std::isfinite(exponent) || std::abs(exponent) > 1000.0 || output.offset != 0.0) {
      error = "Invalid unit exponent; affine temperature units cannot be raised to a power.";
      return false;
    }
    position = static_cast<std::size_t>(parsed.ptr - text.data());
    output.scale = std::pow(output.scale, exponent);
    for (double& item : output.dimension) {
      item *= exponent;
    }
    output.role = UnitRole::Linear;
  }
  return true;
}

bool parse_product(const std::string& text, std::size_t& position, const std::map<std::string, UnitDefinition>& definitions, UnitDefinition& output, std::string& error, int depth) {
  if (!parse_factor(text, position, definitions, output, error, depth)) {
    return false;
  }
  while (position < text.size()) {
    skip_space(text, position);
    if (position == text.size() || text[position] == ')') {
      break;
    }
    const char operation = text[position];
    if (operation != '*' && operation != '/') {
      error = "Expected '*' or '/' in unit expression.";
      return false;
    }
    ++position;
    UnitDefinition right;
    if (!parse_factor(text, position, definitions, right, error, depth)) {
      return false;
    }
    if (output.offset != 0.0 || right.offset != 0.0) {
      error = "Use delta_degC for temperature differences in compound units.";
      return false;
    }
    double sign = 1.0;
    if (operation == '/') {
      sign = -1.0;
      output.scale /= right.scale;
    } else {
      output.scale *= right.scale;
    }
    for (std::size_t index = 0; index < output.dimension.size(); ++index) {
      output.dimension[index] += sign * right.dimension[index];
    }
    output.role = UnitRole::Linear;
  }
  return true;
}

std::vector<UserOption> option_definitions(const CoreFrameworkUnits& units) {
  std::vector<UserOption> result;
  const auto systems = units.system_ids();
  result.emplace_back("units/default_system", UserOptionKind::Text, std::string("N-mm-s"), [systems](const UserOptionValue& value) {
    const auto* id = std::get_if<std::string>(&value);
    return id != nullptr && std::find(systems.begin(), systems.end(), *id) != systems.end();
  });
  const WorkspaceUnitContext defaults;
  for (const auto& id : units.quantity_ids()) {
    std::string value;
    const auto found = defaults.display_units.find(id);
    if (found != defaults.display_units.end()) {
      value = found->second;
    }
    result.emplace_back("units/display/" + id, UserOptionKind::Text, value, [&units, id](const UserOptionValue& input) {
      const auto* expression = std::get_if<std::string>(&input);
      if (expression == nullptr) {
        return false;
      }
      if (expression->empty()) {
        return true;
      }
      QuantityUnit quantity;
      std::string error;
      double output = 0.0;
      return units.quantity(id, quantity, error) && units.to_si(0.0, *expression, quantity.dimension, quantity.role, output, error);
    });
  }
  return result;
}

void temperature_role(UnitDefinition& definition, UnitRole role) {
  if (role == UnitRole::TemperatureDifference && definition.role == UnitRole::AbsoluteTemperature) {
    definition.role = UnitRole::TemperatureDifference;
    definition.offset = 0.0;
  }
}

std::string power_term(const std::string& unit, double exponent) {
  std::ostringstream output;
  output.imbue(std::locale::classic());
  output << unit;
  if (exponent != 1.0) {
    output << '^' << std::setprecision(17) << exponent;
  }
  return output.str();
}
}

SimUnits::CoreFrameworkUnits::CoreFrameworkUnits() {
  systems.emplace("SI", UnitSystem("SI", "m", "N", "s", 1.0, 1.0, 1.0));
  systems.emplace("N-mm-s", UnitSystem("N-mm-s", "mm", "N", "s", 1e-3, 1.0, 1.0));
  systems.emplace("kN-m-s", UnitSystem("kN-m-s", "m", "kN", "s", 1.0, 1e3, 1.0));
  systems.emplace("N-mm-ms", UnitSystem("N-mm-ms", "mm", "N", "ms", 1e-3, 1.0, 1e-3));
  systems.emplace("kN-mm-ms", UnitSystem("kN-mm-ms", "mm", "kN", "ms", 1e-3, 1e3, 1e-3));
  add_unit(UnitDefinition("1", Dimensionless, 1.0));
  add_unit(UnitDefinition("%", Dimensionless, 0.01));
  add_unit(UnitDefinition("kg", Mass, 1.0));
  add_unit(UnitDefinition("g", Mass, 1e-3));
  add_unit(UnitDefinition("tonne", Mass, 1e3));
  add_unit(UnitDefinition("t", Mass, 1e3));
  add_unit(UnitDefinition("m", Length, 1.0));
  add_unit(UnitDefinition("mm", Length, 1e-3));
  add_unit(UnitDefinition("cm", Length, 1e-2));
  add_unit(UnitDefinition("in", Length, 0.0254));
  add_unit(UnitDefinition("ft", Length, 0.3048));
  add_unit(UnitDefinition("s", Time, 1.0));
  add_unit(UnitDefinition("ms", Time, 1e-3));
  add_unit(UnitDefinition("min", Time, 60.0));
  add_unit(UnitDefinition("N", Force, 1.0));
  add_unit(UnitDefinition("kN", Force, 1e3));
  add_unit(UnitDefinition("Pa", Stress, 1.0));
  add_unit(UnitDefinition("kPa", Stress, 1e3));
  add_unit(UnitDefinition("MPa", Stress, 1e6));
  add_unit(UnitDefinition("GPa", Stress, 1e9));
  add_unit(UnitDefinition("J", Energy, 1.0));
  add_unit(UnitDefinition("mJ", Energy, 1e-3));
  add_unit(UnitDefinition("kJ", Energy, 1e3));
  add_unit(UnitDefinition("W", Power, 1.0));
  add_unit(UnitDefinition("mW", Power, 1e-3));
  add_unit(UnitDefinition("kW", Power, 1e3));
  add_unit(UnitDefinition("Hz", InverseTime, 1.0));
  add_unit(UnitDefinition("K", Temperature, 1.0, UnitRole::AbsoluteTemperature));
  add_unit(UnitDefinition("degC", Temperature, 1.0, UnitRole::AbsoluteTemperature, 273.15));
  add_unit(UnitDefinition("degF", Temperature, 5.0 / 9.0, UnitRole::AbsoluteTemperature, 273.15 - 32.0 * 5.0 / 9.0));
  add_unit(UnitDefinition("delta_K", Temperature, 1.0, UnitRole::TemperatureDifference));
  add_unit(UnitDefinition("delta_degC", Temperature, 1.0, UnitRole::TemperatureDifference));
  add_unit(UnitDefinition("delta_degF", Temperature, 5.0 / 9.0, UnitRole::TemperatureDifference));
  add_unit(UnitDefinition("rad", Dimensionless, 1.0, UnitRole::Angle));
  add_unit(UnitDefinition("deg", Dimensionless, std::acos(-1.0) / 180.0, UnitRole::Angle));
  add_unit(UnitDefinition("A", {0, 0, 0, 0, 1, 0, 0}, 1.0));
  add_unit(UnitDefinition("mA", {0, 0, 0, 0, 1, 0, 0}, 1e-3));
  add_unit(UnitDefinition("mol", {0, 0, 0, 0, 0, 1, 0}, 1.0));
  add_unit(UnitDefinition("cd", {0, 0, 0, 0, 0, 0, 1}, 1.0));
  add_unit(UnitDefinition("V", {1, 2, -3, 0, -1, 0, 0}, 1.0));
  add_unit(UnitDefinition("ohm", {1, 2, -3, 0, -2, 0, 0}, 1.0));
  add_unit(UnitDefinition("S", {-1, -2, 3, 0, 2, 0, 0}, 1.0));
  add_unit(UnitDefinition("T", {1, 0, -2, 0, -1, 0, 0}, 1.0));
  add_unit(UnitDefinition("H", {1, 2, -2, 0, -2, 0, 0}, 1.0));
  add_unit(UnitDefinition("F", {-1, -2, 4, 0, 2, 0, 0}, 1.0));
  add_quantity(QuantityUnit("dimensionless", "1", Dimensionless));
  add_quantity(QuantityUnit("relative_error", "%", Dimensionless));
  add_quantity(QuantityUnit("phase_angle", "deg", Dimensionless, UnitRole::Angle));
  add_quantity(QuantityUnit("mass_flow", "kg/s", {1, 0, -1, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("angular_velocity_squared", "1/s^2", {0, 0, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("electric_potential", "V", {1, 2, -3, 0, -1, 0, 0}));
  add_quantity(QuantityUnit("electric_field", "V/m", {1, 1, -3, 0, -1, 0, 0}));
  add_quantity(QuantityUnit("electric_current_density", "A/m^2", {0, -2, 0, 0, 1, 0, 0}));
  add_quantity(QuantityUnit("magnetic_flux_density", "T", {1, 0, -2, 0, -1, 0, 0}));
  add_quantity(QuantityUnit("stress_intensity", "Pa*m^0.5", {1, -0.5, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("turbulent_kinetic_energy", "m^2/s^2", {0, 2, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("turbulence_density_rate", "kg/(m^3*s)", {1, -3, -1, 0, 0, 0, 0}));
  // Cycle count is dimensionless; the quantity identifies length per cycle.
  add_quantity(QuantityUnit("crack_growth_per_cycle", "m", Length));
  add_quantity(QuantityUnit("strain", "1", Dimensionless));
  add_quantity(QuantityUnit("poisson_ratio", "1", Dimensionless));
  add_quantity(QuantityUnit("mass", "kg", Mass));
  add_quantity(QuantityUnit("length", "m", Length));
  add_quantity(QuantityUnit("area", "m^2", {0, 2, 0, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("volume", "m^3", {0, 3, 0, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("area_moment", "m^4", {0, 4, 0, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("mass_moment", "kg*m^2", {1, 2, 0, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("time", "s", Time));
  add_quantity(QuantityUnit("frequency", "Hz", InverseTime));
  add_quantity(QuantityUnit("rate", "1/s", InverseTime));
  add_quantity(QuantityUnit("inverse_time", "1/s", InverseTime));
  add_quantity(QuantityUnit("frequency_squared", "1/s^2", {0, 0, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("velocity", "m/s", {0, 1, -1, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("acceleration", "m/s^2", {0, 1, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("angle", "rad", Dimensionless, UnitRole::Angle));
  add_quantity(QuantityUnit("angular_velocity", "rad/s", InverseTime));
  add_quantity(QuantityUnit("force", "N", Force));
  add_quantity(QuantityUnit("moment", "N*m", Energy));
  add_quantity(QuantityUnit("stress", "Pa", Stress));
  add_quantity(QuantityUnit("pressure", "Pa", Stress));
  add_quantity(QuantityUnit("inverse_stress", "1/Pa", InverseStress));
  add_quantity(QuantityUnit("density", "kg/m^3", Density));
  add_quantity(QuantityUnit("energy", "J", Energy));
  add_quantity(QuantityUnit("energy_density", "J/m^3", Stress));
  add_quantity(QuantityUnit("power", "W", Power));
  add_quantity(QuantityUnit("force_per_length", "N/m", {1, 0, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("force_per_volume", "N/m^3", {1, -2, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("damping", "N*s/m", {1, 0, -1, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("temperature", "K", Temperature, UnitRole::AbsoluteTemperature));
  add_quantity(QuantityUnit("temperature_difference", "delta_K", Temperature, UnitRole::TemperatureDifference));
  add_quantity(QuantityUnit("expansion", "1/K", Expansion));
  add_quantity(QuantityUnit("specific_heat", "J/(kg*K)", SpecificHeat));
  add_quantity(QuantityUnit("conductivity", "W/(m*K)", Conductivity));
  add_quantity(QuantityUnit("heat_flux", "W/m^2", {1, 0, -3, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("body_heat_flux", "W/m^3", {1, -1, -3, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("heat_transfer", "W/(m^2*K)", {1, 0, -3, -1, 0, 0, 0}));
  add_quantity(QuantityUnit("newton_gravity", "m^3/(kg*s^2)", {-1, 3, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("modal_force", "kg^0.5*m/s^2", {0.5, 1, -2, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("stefan_boltzmann", "W/(m^2*K^4)", {1, 0, -3, -4, 0, 0, 0}));
  add_quantity(QuantityUnit("dynamic_viscosity", "Pa*s", {1, -1, -1, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("kinematic_viscosity", "m^2/s", {0, 2, -1, 0, 0, 0, 0}));
  add_quantity(QuantityUnit("electric_conductivity", "S/m", {-1, -3, 3, 0, 2, 0, 0}));
  add_quantity(QuantityUnit("magnetic_permeability", "H/m", {1, 1, -2, 0, -2, 0, 0}));
  add_quantity(QuantityUnit("permittivity", "F/m", {-1, -3, 4, 0, 2, 0, 0}));
  for (const std::string workspace : {"cubit", "calculix", "aster", "opencourant", "opencfs", "openfoam", "postprocessor", "report"}) {
    defaults.emplace(workspace, WorkspaceUnitContext(workspace));
  }
  reset_project();
}

void SimUnits::CoreFrameworkUnits::add_unit(UnitDefinition definition) {
  const auto id = definition.id;
  definitions.emplace(id, std::move(definition));
}
void SimUnits::CoreFrameworkUnits::add_quantity(QuantityUnit definition) {
  const auto id = definition.quantity;
  quantities.emplace(id, std::move(definition));
}
std::vector<std::string> SimUnits::CoreFrameworkUnits::workspace_ids() const {
  std::vector<std::string> result;
  for (const auto& item : contexts) {
    result.push_back(item.first);
  }
  return result;
}
std::vector<std::string> SimUnits::CoreFrameworkUnits::system_ids() const {
  return {"SI", "N-mm-s", "kN-m-s", "N-mm-ms", "kN-mm-ms"};
}
std::vector<std::string> SimUnits::CoreFrameworkUnits::quantity_ids() const {
  std::vector<std::string> result;
  for (const auto& item : quantities) {
    result.push_back(item.first);
  }
  return result;
}
bool SimUnits::CoreFrameworkUnits::initialize(UserOptionsInterface& store, std::string& error) {
  const auto schema = option_definitions(*this);
  auto candidate = defaults;
  for (auto& item : candidate) {
    UserOptionValues values;
    if (!store.load(item.first, schema, values, error)) {
      return false;
    }
    item.second.system = std::get<std::string>(values.at("units/default_system"));
    item.second.display_units.clear();
    for (const auto& id : quantity_ids()) {
      const auto& value = std::get<std::string>(values.at("units/display/" + id));
      if (!value.empty()) {
        item.second.display_units.emplace(id, value);
      }
    }
  }
  defaults = std::move(candidate);
  options = &store;
  reset_project();
  return true;
}
void SimUnits::CoreFrameworkUnits::shutdown() noexcept {
  options = nullptr;
}
void SimUnits::CoreFrameworkUnits::swap_project_state(CoreFrameworkUnits& other) noexcept {
  contexts.swap(other.contexts);
  geometryUnit.swap(other.geometryUnit);
  std::swap(changeGeneration, other.changeGeneration);
}
void SimUnits::CoreFrameworkUnits::reset_project() {
  contexts = defaults;
  geometryUnit.clear();
  ++changeGeneration;
}
std::uint64_t SimUnits::CoreFrameworkUnits::generation() const noexcept {
  return changeGeneration;
}
bool SimUnits::CoreFrameworkUnits::reset_workspace(const std::string& workspace, std::string& error) {
  error.clear();
  const auto found = defaults.find(workspace);
  if (found == defaults.end()) {
    error = "Unknown workspace: " + workspace;
    return false;
  }
  contexts[workspace] = found->second;
  ++changeGeneration;
  return true;
}
bool SimUnits::CoreFrameworkUnits::restore_project(const std::vector<WorkspaceUnitContext>& saved, const std::string& geometry, std::string& error) {
  auto candidate = *this;
  candidate.reset_project();
  std::vector<std::string> seen;
  for (const auto& context : saved) {
    if (std::find(seen.begin(), seen.end(), context.workspace) != seen.end()) {
      error = "Duplicate saved workspace unit context.";
      return false;
    }
    if (!candidate.set_workspace_context(context, error)) {
      return false;
    }
    seen.push_back(context.workspace);
  }
  if (!geometry.empty() && !candidate.declare_geometry_unit(geometry, error)) {
    return false;
  }
  contexts = std::move(candidate.contexts);
  geometryUnit = std::move(candidate.geometryUnit);
  ++changeGeneration;
  error.clear();
  return true;
}
bool SimUnits::CoreFrameworkUnits::save_defaults(const std::string& workspace, std::string& error) {
  WorkspaceUnitContext context;
  if (!workspace_context(workspace, context, error)) {
    return false;
  }
  if (options == nullptr) {
    error = "Framework user-options storage is not initialized.";
    return false;
  }
  UserOptionValues values;
  values.emplace("units/default_system", context.system);
  for (const auto& id : quantity_ids()) {
    std::string value;
    const auto found = context.display_units.find(id);
    if (found != context.display_units.end()) {
      value = found->second;
    }
    values.emplace("units/display/" + id, value);
  }
  if (!options->save(workspace, option_definitions(*this), values, error)) {
    return false;
  }
  context.native_system.clear();
  defaults[workspace] = context;
  return true;
}

bool SimUnits::CoreFrameworkUnits::unit(const std::string& expression, UnitDefinition& output, std::string& error) const {
  error.clear();
  if (expression.empty() || expression.size() > 256) {
    error = "A nonempty unit expression of at most 256 characters is required.";
    return false;
  }
  std::size_t position = 0;
  UnitDefinition candidate;
  if (!parse_product(expression, position, definitions, candidate, error, 0)) {
    return false;
  }
  skip_space(expression, position);
  if (position != expression.size() || !std::isfinite(candidate.scale) || candidate.scale <= 0.0) {
    error = "Invalid or out-of-range unit expression.";
    return false;
  }
  candidate.id = expression;
  output = std::move(candidate);
  return true;
}
bool SimUnits::CoreFrameworkUnits::quantity(const std::string& id, QuantityUnit& output, std::string& error) const {
  error.clear();
  const auto found = quantities.find(id);
  if (found == quantities.end()) {
    error = "Unknown physical quantity: " + id;
    return false;
  }
  output = found->second;
  return true;
}
bool SimUnits::CoreFrameworkUnits::system_unit(const std::string& system, const std::string& quantityId, QuantityUnit& output, std::string& error) const {
  QuantityUnit candidate;
  if (!quantity(quantityId, candidate, error)) {
    return false;
  }
  const auto found = systems.find(system);
  if (found == systems.end()) {
    error = "Unknown unit-system preset: " + system;
    return false;
  }
  if (quantityId == "relative_error")
  {
    // CCX error values are native percentages, not unscaled fractions.
    output = candidate;
    return true;
  }
  const auto& preset = found->second;
  if (candidate.role == UnitRole::AbsoluteTemperature || candidate.role == UnitRole::TemperatureDifference || candidate.role == UnitRole::Angle) {
    output = candidate;
    return true;
  }
  const auto& d = candidate.dimension;
  const std::array<double, 7> exponents = {d[0], d[1] - d[0], d[2] + 2.0 * d[0], d[3], d[4], d[5], d[6]};
  const std::array<std::string, 7> symbols = {preset.force_unit, preset.length_unit, preset.time_unit, "K", "A", "mol", "cd"};
  std::string expression;
  for (std::size_t index = 0; index < symbols.size(); ++index) {
    if (exponents[index] == 0.0) {
      continue;
    }
    if (!expression.empty()) {
      expression += '*';
    }
    expression += power_term(symbols[index], exponents[index]);
  }
  if (expression.empty()) {
    expression = "1";
  }
  if (quantityId == "length") {
    expression = preset.length_unit;
  } else if (quantityId == "time") {
    expression = preset.time_unit;
  } else if (quantityId == "stress" || quantityId == "pressure") {
    expression = preset.force_unit + "/" + preset.length_unit + "^2";
    if (system == "SI") {
      expression = "Pa";
    }
  } else if (quantityId == "mass" || quantityId == "density") {
    std::string mass = "kg";
    if (system == "N-mm-s" || system == "kN-m-s") {
      mass = "tonne";
    } else if (system == "N-mm-ms") {
      mass = "g";
    }
    expression = mass;
    if (quantityId == "density") {
      expression += "/" + preset.length_unit + "^3";
    }
  }
  candidate.unit = expression;
  output = std::move(candidate);
  return true;
}

bool SimUnits::CoreFrameworkUnits::workspace_context(const std::string& workspace, WorkspaceUnitContext& output, std::string& error) const {
  error.clear();
  const auto found = contexts.find(workspace);
  if (found == contexts.end()) {
    error = "Unknown workspace: " + workspace;
    return false;
  }
  output = found->second;
  return true;
}
bool SimUnits::CoreFrameworkUnits::validate_workspace_context(const WorkspaceUnitContext& context, std::string& error) const
{
  error.clear();
  if (contexts.find(context.workspace) == contexts.end() || systems.find(context.system) == systems.end())
  {
    error = "Unknown workspace or unit-system preset.";
    return false;
  }
  for (const auto& item : context.display_units)
  {
    QuantityUnit definition;
    double result = 0.0;
    if (!quantity(item.first, definition, error) || !to_si(0.0, item.second, definition.dimension, definition.role, result, error))
    {
      return false;
    }
  }
  return true;
}

bool SimUnits::CoreFrameworkUnits::set_workspace_context(const WorkspaceUnitContext& context, std::string& error)
{
  if (!validate_workspace_context(context, error))
  {
    return false;
  }
  contexts[context.workspace] = context;
  ++changeGeneration;
  return true;
}
bool SimUnits::CoreFrameworkUnits::set_workspace_system(const std::string& workspace, const std::string& system, std::string& error) {
  WorkspaceUnitContext context;
  if (!workspace_context(workspace, context, error)) {
    return false;
  }
  context.system = system;
  return set_workspace_context(context, error);
}
bool SimUnits::CoreFrameworkUnits::display_unit(const std::string& workspace, const std::string& quantityId, std::string& output, std::string& error) const {
  WorkspaceUnitContext context;
  QuantityUnit definition;
  if (!workspace_context(workspace, context, error) || !quantity(quantityId, definition, error)) {
    return false;
  }
  const auto found = context.display_units.find(quantityId);
  if (found != context.display_units.end()) {
    output = found->second;
    return true;
  }
  if (quantityId == "frequency")
  {
    // Use the quantity's Hz default for presentation, independently of the
    // coherent solver time unit. Explicit display preferences take priority.
    output = definition.unit;
    return true;
  }
  if (!system_unit(context.system, quantityId, definition, error)) {
    return false;
  }
  output = definition.unit;
  return true;
}
bool SimUnits::CoreFrameworkUnits::set_display_unit(const std::string& workspace, const std::string& quantityId, const std::string& expression, std::string& error) {
  WorkspaceUnitContext context;
  QuantityUnit definition;
  if (!workspace_context(workspace, context, error) || !quantity(quantityId, definition, error)) {
    return false;
  }
  if (expression.empty()) {
    context.display_units.erase(quantityId);
  } else {
    context.display_units[quantityId] = expression;
  }
  return set_workspace_context(context, error);
}
bool SimUnits::CoreFrameworkUnits::validate_source(const SourceUnitContext& context, std::string& error) const {
  error.clear();
  if (!context.system.empty() && systems.find(context.system) == systems.end()) {
    error = "Unknown result-source unit system.";
    return false;
  }
  double value = 0.0;
  if (!context.coordinate_unit.empty() && !to_si(0.0, context.coordinate_unit, Length, UnitRole::Linear, value, error)) {
    return false;
  }
  if (!context.time_unit.empty() && !to_si(0.0, context.time_unit, Time, UnitRole::Linear, value, error)) {
    return false;
  }
  return true;
}
bool SimUnits::CoreFrameworkUnits::declare_geometry_unit(const std::string& expression, std::string& error) {
  double value = 0.0;
  if (!to_si(0.0, expression, Length, UnitRole::Linear, value, error)) {
    return false;
  }
  if (!geometryUnit.empty() && geometryUnit != expression) {
    error = "Cubit geometry units are already declared; changing workspace units does not rescale geometry.";
    return false;
  }
  geometryUnit = expression;
  ++changeGeneration;
  return true;
}
const std::string& SimUnits::CoreFrameworkUnits::geometry_unit() const noexcept {
  return geometryUnit;
}
bool SimUnits::CoreFrameworkUnits::geometry_to_si(double value, double& output, std::string& error) const {
  return to_si(value, geometryUnit, Length, UnitRole::Linear, output, error);
}

bool SimUnits::CoreFrameworkUnits::compatible(const UnitDefinition& source, const UnitDefinition& target) {
  if (source.role != target.role) {
    return false;
  }
  for (std::size_t index = 0; index < source.dimension.size(); ++index) {
    const double left = source.dimension[index];
    const double right = target.dimension[index];
    if (!std::isfinite(left) || !std::isfinite(right) || std::abs(left - right) > 1e-12 * std::max({1.0, std::abs(left), std::abs(right)})) {
      return false;
    }
  }
  return true;
}
bool SimUnits::CoreFrameworkUnits::apply(double value, const UnitDefinition& source, const UnitDefinition& target, double& output, std::string& error) {
  error.clear();
  if (!std::isfinite(value) || !compatible(source, target)) {
    error = "Unit conversion requires a finite value and compatible physical quantities.";
    return false;
  }
  if (!std::isfinite(source.scale) || !std::isfinite(target.scale) || source.scale <= 0.0 || target.scale <= 0.0 || !std::isfinite(source.offset) || !std::isfinite(target.offset)) {
    error = "Invalid unit conversion scale or offset.";
    return false;
  }
  if ((source.offset != 0.0 || target.offset != 0.0) && source.role != UnitRole::AbsoluteTemperature) {
    error = "Only absolute temperatures allow an affine conversion.";
    return false;
  }
  const double canonical = value * source.scale + source.offset;
  const double result = (canonical - target.offset) / target.scale;
  if (!std::isfinite(canonical) || !std::isfinite(result)) {
    error = "Unit conversion exceeded the supported numeric range.";
    return false;
  }
  output = result;
  return true;
}
bool SimUnits::CoreFrameworkUnits::convert(double value, const std::string& from, const std::string& to, double& output, std::string& error) const {
  UnitDefinition source;
  UnitDefinition target;
  return unit(from, source, error) && unit(to, target, error) && apply(value, source, target, output, error);
}
bool SimUnits::CoreFrameworkUnits::read(double value, const std::string& expression, const Dimension& dimension, UnitRole role, double& output, std::string& error) const {
  return to_si(value, expression, dimension, role, output, error);
}
bool SimUnits::CoreFrameworkUnits::to_si(double value, const std::string& expression, const Dimension& dimension, UnitRole role, double& output, std::string& error) const {
  UnitDefinition source;
  if (!unit(expression, source, error)) {
    return false;
  }
  temperature_role(source, role);
  const UnitDefinition target("canonical", dimension, 1.0, role);
  return apply(value, source, target, output, error);
}
bool SimUnits::CoreFrameworkUnits::from_si(double value, const std::string& expression, const Dimension& dimension, UnitRole role, double& output, std::string& error) const {
  UnitDefinition target;
  if (!unit(expression, target, error)) {
    return false;
  }
  temperature_role(target, role);
  const UnitDefinition source("canonical", dimension, 1.0, role);
  return apply(value, source, target, output, error);
}
bool SimUnits::CoreFrameworkUnits::system_to_si(double value, const Dimension& dimension, const std::string& system, double& output, std::string& error) const {
  error.clear();
  const auto found = systems.find(system);
  if (found == systems.end() || !std::isfinite(value)) {
    error = "A known source unit system and finite value are required.";
    return false;
  }
  double scale = 1.0;
  if (!found->second.scale_to_si(dimension, scale, error)) {
    return false;
  }
  const double result = value * scale;
  if (!std::isfinite(result)) {
    error = "Source unit conversion exceeded the supported numeric range.";
    return false;
  }
  output = result;
  return true;
}
bool SimUnits::CoreFrameworkUnits::system_from_si(double value, const Dimension& dimension, const std::string& system, double& output, std::string& error) const {
  error.clear();
  const auto found = systems.find(system);
  if (found == systems.end() || !std::isfinite(value)) {
    error = "A known destination unit system and finite value are required.";
    return false;
  }
  double scale = 1.0;
  if (!found->second.scale_to_si(dimension, scale, error)) {
    return false;
  }
  const double result = value / scale;
  if (!std::isfinite(result)) {
    error = "Destination unit conversion exceeded the supported numeric range.";
    return false;
  }
  output = result;
  return true;
}
bool SimUnits::CoreFrameworkUnits::to_solver(double value, const Dimension& dimension, const std::string& workspace, double& output, std::string& error) const {
  WorkspaceUnitContext context;
  return workspace_context(workspace, context, error) && system_from_si(value, dimension, context.system, output, error);
}
bool SimUnits::CoreFrameworkUnits::from_solver(double value, const Dimension& dimension, const std::string& workspace, double& output, std::string& error) const {
  WorkspaceUnitContext context;
  return workspace_context(workspace, context, error) && system_to_si(value, dimension, context.system, output, error);
}
SimUnits::CoreFrameworkUnits& framework_units() {
  static SimUnits::CoreFrameworkUnits instance;
  return instance;
}
