#include "Core/steps/CoreStepsStructs.hpp"
#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/units/CalculiXDomainUnits.hpp"
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
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "Core/sections/CoreSectionsPersistence.hpp"
#include "Core/sections/CoreOrientationsPersistence.hpp"
#include "Core/interactions/CoreInteractionsPersistence.hpp"
#include "Core/steps/CoreStepsPersistence.hpp"
#include "Core/steps/CoreInitialConditionsPersistence.hpp"
#include "Core/steps/CoreStepSettingsPersistence.hpp"
#include "Core/amplitudes/CoreAmplitudesPersistence.hpp"
#include "Core/loads/CoreLoadsPersistence.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void require(bool value, const std::string& message) {
  if (!value) {
    throw std::runtime_error(message);
  }
}
void near(double actual, double expected) {
  require(std::isfinite(actual) && std::abs(actual - expected) <= std::max(1e-13, std::abs(expected) * 1e-11), "Physical value changed");
}
template<class Model, class Value>
void conversions(const Model& source, Value value, const std::string& quantity, bool inverse = false) {
  std::string error;
  SimUnits::QuantityUnit definition;
  require(framework_units().quantity(quantity, definition, error), error);
  const double original = value(source);
  for (const std::string system : {"SI", "N-mm-s", "kN-m-s", "N-mm-ms", "kN-mm-ms"}) {
    require(framework_units().set_workspace_system("calculix", system, error), error);
    near(value(source), original);
    auto solver = source;
    require(CalculiXDomainUnits::convert(solver, CalculiXDomainUnitView::Solver, error), error);
    double scale = 0.0;
    require(framework_units().to_solver(1.0, definition.dimension, "calculix", scale, error), error);
    double expected = original * scale;
    if (inverse) {
      expected = original / scale;
    }
    near(value(solver), expected);
    auto display = source;
    require(CalculiXDomainUnits::convert(display, CalculiXDomainUnitView::Display, error), error);
    require(CalculiXDomainUnits::convert(display, CalculiXDomainUnitView::Input, error), error);
    require(!display.domain_unit_view, "Input must return canonical data");
    near(value(display), original);
    auto report = source;
    require(CalculiXDomainUnits::convert(report, CalculiXDomainUnitView::Report, error), error);
    double reportValue = 0.0;
    std::string reportUnit;
    require(framework_units().display_unit("report", quantity, reportUnit, error), error);
    require(framework_units().from_si(1.0, reportUnit, definition.dimension, definition.role, reportValue, error), error);
    expected = original * reportValue;
    if (inverse) {
      expected = original / reportValue;
    }
    near(value(report), expected);
  }
}
template<class Persistence, class Model, class Data>
void persistence(const Model& source, Data data, const std::string& filename) {
  {
    HDF5Tool file(filename);
    require(Persistence::write(file, source), "Save domain fixture");
    require(file.close(), file.last_error());
  }
  Model restored;
  {
    HDF5Tool file(filename, false);
    require(Persistence::read(file, restored), "Restore domain fixture");
    require(file.close(), file.last_error());
  }
  require(data(restored) == data(source), "Persistence changed canonical records");
  require(!restored.domain_unit_view, "Restored data must be canonical");
}
void domains(const std::string& filename) {
  std::string error;
  framework_units().reset_project();
  require(framework_units().declare_geometry_unit("mm", error), error);
  require(framework_units().set_workspace_system("report", "SI", error), error);
  CoreSections sections;
  sections.sections_data = {{7, 2, 13}};
  sections.shell_section_data = {{"13", "42", "9", "-1", "0.002", "0.3"}};
  conversions(sections, [](const auto& m) { return std::stod(m.shell_section_data[0][4]); }, "length");
  persistence<CoreSectionsPersistence>(sections, [](const auto& m) { return m.shell_section_data; }, filename);
  auto invalid = sections;
  invalid.shell_section_data[0][4] = "invalid";
  const auto before = invalid.shell_section_data;
  require(!CalculiXDomainUnits::convert(invalid, CalculiXDomainUnitView::Display, error), "Reject invalid values");
  require(before == invalid.shell_section_data && !invalid.domain_unit_view, "Failed conversion changed data");

  {
    HDF5Tool file(filename);
    require(file.ensureGroup("Invalid"), "Create malformed fixture");
    require(file.close(), file.last_error());
  }
  // A corrupt unit marker must not replace a populated model.
  {
    HDF5Tool file(filename);
    require(CoreSectionsPersistence::write(file, sections), "Save section fixture");
    require(file.write_dataset_string_rank_1("value_units", CalculiXModelSchema::path("Sections"), {"invalid"}), "Write invalid source units");
    require(file.close(), file.last_error());
  }
  {
    auto retained = sections;
    HDF5Tool file(filename, false);
    require(!CoreSectionsPersistence::read(file, retained), "Reject invalid source units");
    require(retained.sections_data == sections.sections_data && retained.shell_section_data == sections.shell_section_data, "Failed read replaced live records");
    require(file.close(), file.last_error());
  }

  CoreOrientations orientations;
  orientations.a_data = {{"2", "0.02", "0", "0"}};
  orientations.b_data = {{"3", "0", "0.03", "0"}};
  orientations.rotation_data = {{"4", "1.5707963267948966"}};
  conversions(orientations, [](const auto& m) { return std::stod(m.a_data[0][1]); }, "length");
  persistence<CoreOrientationsPersistence>(orientations, [](const auto& m) { return m.rotation_data; }, filename);
  auto geometry = orientations;
  require(CalculiXDomainUnits::convert(geometry, CalculiXDomainUnitView::Geometry, error), error);
  near(std::stod(geometry.a_data[0][1]), 20.0);
  near(std::stod(geometry.rotation_data[0][1]), 90.0);

  CoreConstraints constraints;
  constraints.tie_constraint_data = {{"2", "4", "5", "", "0.001"}};
  constraints.equation_data = {{3, 10, 1, 2.0}, {3, 11, 6, -4.0}};
  conversions(constraints, [](const auto& m) { return std::stod(m.tie_constraint_data[0][4]); }, "length");
  conversions(constraints, [](const auto& m) { return m.equation_data[0][3]; }, "length", true);
  persistence<CoreInteractionsPersistence>(constraints, [](const auto& m) { return m.equation_data; }, filename);

  auto invalidEquation = constraints;
  invalidEquation.equation_data[0][2] = 1.5;
  const auto equationBefore = invalidEquation.equation_data;
  require(!CalculiXDomainUnits::convert(invalidEquation, CalculiXDomainUnitView::Solver, error), "Reject fractional equation DOF");
  require(invalidEquation.equation_data == equationBefore, "Rejected equation conversion changed stored values");

  CoreContactPairs contacts;
  contacts.adjust_contactpair_data = {{"8", "0.003", "-1"}};
  conversions(contacts, [](const auto& m) { return std::stod(m.adjust_contactpair_data[0][1]); }, "length");
  persistence<CoreInteractionsPersistence>(contacts, [](const auto& m) { return m.adjust_contactpair_data; }, filename);

  CoreSurfaceInteractions interactions;
  interactions.linear_surfacebehavior_data = {{"3", "1e12", "1e6", "0.0001"}};
  interactions.gap_conductance_data = {{"4", "100", "0", "273.15"}};
  interactions.friction_data = {{"5", "0.2", "1e10"}};
  conversions(interactions, [](const auto& m) { return std::stod(m.linear_surfacebehavior_data[0][1]); }, "force_per_volume");
  conversions(interactions, [](const auto& m) { return std::stod(m.gap_conductance_data[0][1]); }, "heat_transfer");
  persistence<CoreInteractionsPersistence>(interactions, [](const auto& m) { return m.gap_conductance_data; }, filename);

  CoreSteps steps;
  steps.static_data = {{"5", "", "", "", "", "0", "0.01", "1", "0.001", "0.1", ""}};
  steps.steps_data = {{7, 1, 1, 2, 5, -1, -1, -1, -1}};
  steps.name_data = {{"1", "Static fixture"}};
  steps.parameter_data = {{"1", "", "", "", "", "", "", ""}};
  steps.backup_static_data = steps.static_data;
  steps.has_backup = true;
  conversions(steps, [](const auto& m) { return std::stod(m.static_data[0][6]); }, "time");
  conversions(steps, [](const auto& m) { return std::stod(m.backup_static_data[0][6]); }, "time");
  persistence<CoreStepsPersistence>(steps, [](const auto& m) { return m.static_data; }, filename);
  persistence<CoreStepsPersistence>(steps, [](const auto& m) { return m.steps_data; }, filename);
  require(steps.save_backup(), "Save canonical steps before preparation");
  require(framework_units().set_workspace_system("calculix", "N-mm-ms", error), error);
  const auto split = steps.split_step(7, {{250.0, 750.0}});
  require(split.valid && split.pieces.size() == 3, "Split step with millisecond trajectory intervals");
  for (std::size_t index = 0; index < split.pieces.size(); ++index) {
    const int record = steps.get_steps_data_id_from_step_id(split.pieces[index].step_id);
    require(record >= 0, "Retain generated step IDs");
    const int procedure = steps.get_static_data_id_from_static_id(steps.steps_data[record][4]);
    require(procedure >= 0, "Retain generated procedure references");
    near(std::stod(steps.static_data[procedure][6]), 0.01);
    near(std::stod(steps.static_data[procedure][7]), (split.pieces[index].end - split.pieces[index].begin) / 1000.0);
  }
  require(steps.load_backup(), "Restore canonical steps after preparation");
  require(steps.steps_data.size() == 1 && steps.steps_data[0][0] == 7, "Restore original step IDs");
  near(std::stod(steps.static_data[0][7]), 1.0);


  CoreInitialConditions initial;
  initial.stress_block_data = {{6, 42, 2e6, 0, 0, 0, 0, 0}};
  conversions(initial, [](const auto& m) { return m.stress_block_data[0][2]; }, "stress");
  persistence<CoreInitialConditionsPersistence>(initial, [](const auto& m) { return m.stress_block_data; }, filename);

  CoreDamping damping;
  damping.damping_data = {"0.2", "0.003"};
  conversions(damping, [](const auto& m) { return std::stod(m.damping_data[0]); }, "inverse_time");
  conversions(damping, [](const auto& m) { return std::stod(m.damping_data[1]); }, "time");
  persistence<CoreStepSettingsPersistence>(damping, [](const auto& m) { return m.damping_data; }, filename);

  CorePhysicalConstants constants;
  constants.physicalconstants_data = {"0", "5.670374419e-8", "6.67430e-11"};
  conversions(constants, [](const auto& m) { return std::stod(m.physicalconstants_data[1]); }, "stefan_boltzmann");
  conversions(constants, [](const auto& m) { return std::stod(m.physicalconstants_data[2]); }, "newton_gravity");
  persistence<CoreStepSettingsPersistence>(constants, [](const auto& m) { return m.physicalconstants_data; }, filename);

  CoreAmplitudes amplitude;
  amplitude.amplitudes_data = {{5, 5, 5, 5, 1, 5}};
  amplitude.name_amplitude_data = {{"5", "frequency curve"}};
  amplitude.shiftx_amplitude_data = {{"5", "2"}};
  amplitude.shifty_amplitude_data = {{"5", "0"}};
  amplitude.amplitudevalues_amplitude_data = {{5, 12, 0.5}, {5, 24, 1.0}};
  amplitude.axis_quantities[5] = "frequency";
  conversions(amplitude, [](const auto& m) { return m.amplitudevalues_amplitude_data[0][1]; }, "frequency");
  persistence<CoreAmplitudesPersistence>(amplitude, [](const auto& m) { return m.axis_quantities; }, filename);
  persistence<CoreAmplitudesPersistence>(amplitude, [](const auto& m) { return m.amplitudevalues_amplitude_data; }, filename);
  require(amplitude.delete_amplitudes({5}), "Delete generated curve");
  require(amplitude.axis_quantities.empty(), "Delete generated axis metadata");

  CoreLoadsTrajectory trajectory;
  trajectory.heatflux->time_data = {{"1", "0", "0.5"}};
  trajectory.heatflux->radius_data = {{1, 0.002}};
  trajectory.heatflux->magnitude_data = {{1, 2000}};
  trajectory.bodyheatfluxsphere->time_data = {{"2", "0", "1"}};
  trajectory.bodyheatfluxsphere->radius_data = {{2, 0.004}};
  trajectory.bodyheatfluxsphere->depth_data = {{2, 0.003}};
  trajectory.bodyheatfluxsphere->magnitude_data = {{2, 1e6}};
  conversions(*trajectory.heatflux, [](const auto& m) { return m.magnitude_data[0][1]; }, "heat_flux");
  conversions(*trajectory.bodyheatfluxsphere, [](const auto& m) { return m.magnitude_data[0][1]; }, "body_heat_flux");
  {
    HDF5Tool file(filename);
    require(CoreLoadsPersistence::write(file, trajectory), "Save trajectory");
    require(file.close(), file.last_error());
  }
  CoreLoadsTrajectory restored;
  {
    HDF5Tool file(filename, false);
    require(CoreLoadsPersistence::read(file, restored), "Restore trajectory");
    require(file.close(), file.last_error());
  }
  require(trajectory.heatflux->magnitude_data == restored.heatflux->magnitude_data, "Surface trajectory persistence");
  require(trajectory.bodyheatfluxsphere->depth_data == restored.bodyheatfluxsphere->depth_data, "Volume trajectory persistence");
  delete trajectory.heatflux;
  delete trajectory.bodyheatfluxsphere;
  delete restored.heatflux;
  delete restored.bodyheatfluxsphere;
}
}
int main(int argc, char** argv) {
  QCoreApplication application(argc, argv);
  try {
    QTemporaryDir directory;
    require(directory.isValid(), "Create test directory");
    domains(directory.filePath("domains.hdf5").toStdString());
    return 0;
  } catch (const std::exception& exception) {
    PRINT_ERROR("Domain unit regression: %s\n", exception.what());
    return 1;
  }
}
