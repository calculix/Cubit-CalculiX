#include "Core/units/CalculiXDofUnits.hpp"
#include "Core/units/CalculiXDomainUnits.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/jobs/CalculiXJobRunRecord.hpp"
#include "Core/jobs/CoreJobRunPersistence.hpp"
#include "Core/results/CoreResultsSourceMetadata.hpp"
#include "Core/results/CoreResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QTemporaryDir>
#include <QUuid>
#include <cmath>
#include <iostream>

namespace
{
int failures = 0;
void check(bool good, const char* message)
{
  if (!good)
  {
    ++failures;
    std::cerr << message << '\n';
  }
}
void dofs()
{
  std::string error;
  int dof = 0;
  check(CalculiXDofUnits::solver_dof(6, true, 2.0, dof, error) && dof == 3, "ROT NODE moment/rotation maps to solver DOF 3");
  check(CalculiXDofUnits::solver_dof(6, false, 2.0, dof, error) && dof == 6, "Ordinary rotational DOF is retained");
  check(!CalculiXDofUnits::solver_dof(3, true, 2.0, dof, error), "Reject a nonzero translation on a ROT NODE");
  std::string quantity;
  check(CalculiXDofUnits::equation_quantity(2, true, quantity, error) && quantity == "angle", "ROT NODE equations use inverse angle coefficients");
  check(CalculiXDofUnits::equation_quantity(2, false, quantity, error) && quantity == "length", "Ordinary equations use inverse length coefficients");
  CalculiXDofUnits::Boundaries output;
  check(CalculiXDofUnits::boundaries({10, 20}, {20}, {0, 0, 0, 0, 0, 0}, {2, 2, 2, 2, 2, 2}, output, error) && output.size() == 9, "Mixed targets map per node and duplicate zero supports coalesce");
  const auto saved = output;
  check(!CalculiXDofUnits::boundaries({20}, {20}, {0, 0, 0, 1, 0, 0}, {2, 0, 0, 1, 0, 0}, output, error) && output == saved, "Conflicting prescriptions fail transactionally");
  check(CalculiXDofUnits::boundaries({20}, {20}, {0, 0, 0, 0.25, 0, 0}, {0, 0, 0, 1, 0, 0}, output, error) && output.at({20, 1}).second == 0.25, "Prescribed rotation remains in radians");
}
void geometry_units()
{
  std::string error;
  double canonical = 0.0;
  double geometry = 0.0;
  check(framework_units().system_to_si(0.001, SimUnits::Length, "SI", canonical, error) &&
    framework_units().from_si(canonical, "mm", SimUnits::Length, SimUnits::UnitRole::Linear, geometry, error) &&
    std::abs(geometry - 1.0) < 1e-12, "Projection: 0.001 solver metres becomes 1 geometry millimetre");
  check(framework_units().to_si(1000.0, "mm", SimUnits::Length, SimUnits::UnitRole::Linear, canonical, error) &&
    framework_units().from_si(canonical, "m", SimUnits::Length, SimUnits::UnitRole::Linear, geometry, error) &&
    std::abs(geometry - 1.0) < 1e-12, "Centrifugal origin: display millimetres become geometry metres");
}
void solver_number_format()
{
  std::string error;
  if (!framework_units().set_workspace_system("calculix", "N-mm-s", error))
  {
    check(false, "Set solver unit system for numeric format regression");
    return;
  }
  CoreSteps steps;
  steps.static_data = {{"1", "", "", "", "", "0", "0.5", "1", "1e-5", "0.5", ""}};
  const auto original = steps.static_data;
  auto solver = steps;
  if (!CalculiXDomainUnits::convert(solver, CalculiXDomainUnitView::Solver, error))
  {
    check(false, "Convert static solver fields for numeric format regression");
    return;
  }
  check(solver.static_data[0][6] == "5.000000e-01" && solver.static_data[0][7] == "1.000000e+00" &&
    solver.static_data[0][8] == "1.000000e-05" && solver.static_data[0][9] == "5.000000e-01", "Static solver fields use six decimals in scientific notation");
  check(steps.static_data == original, "Solver formatting does not mutate canonical step storage");
  CoreSteps precise;
  precise.static_data = {{"1", "", "", "", "", "0", "0.12345678901234566", "1", "1e-5", "0.5", ""}};
  const double originalIncrement = std::stod(precise.static_data[0][6]);
  if (!framework_units().set_display_unit("calculix", "time", "s", error) ||
      !CalculiXDomainUnits::convert(precise, CalculiXDomainUnitView::Display, error))
  {
    check(false, "Convert display fields for precision regression");
    return;
  }
  check(std::stod(precise.static_data[0][6]) == originalIncrement, "Display conversion preserves full stored precision");
}
void axes_and_fields()
{
  std::string error;
  std::string quantity;
  CoreSteps steps;
  steps.steps_data = {{17, 1, 1, 10, 3}, {22, 1, 1, 2, 1}, {23, 1, 1, 3, 1}};
  steps.steady_state_dynamics_data = {{"3", "", "YES"}};
  check(steps.get_step_axis_quantity(17, quantity, error) && quantity == "frequency", "Harmonic step abscissa is frequency");
  steps.steady_state_dynamics_data[0][2] = "NO";
  check(steps.get_step_axis_quantity(17, quantity, error) && quantity == "time", "Nonharmonic steady-state samples use time");
  check(steps.get_step_axis_quantity(22, quantity, error) && quantity == "time", "Static step uses time");
  check(steps.get_step_axis_quantity(23, quantity, error) && quantity.empty(), "Eigenvalue axis is not guessed");
  SimResults::VtkResultsData results;
  results.source_units.system = "N-mm-s";
  results.source_units.coordinate_unit = "mm";
  results.source_units.time_unit = "s";
  results.increments.emplace_back();
  auto& frame = results.increments.back();
  frame.step = 1;
  frame.mode = -1;
  check(framework_units().set_workspace_system("calculix", "SI", error), "Change current workspace independently of retained result units");
  for (const auto& name : {"FLUX", "RFL", "ENER", "mechanical strains"})
  {
    SimResults::Field field;
    field.name = name;
    field.source = "FRD";
    if (std::string(name) == "mechanical strains")
    {
      field.source = "DAT";
    }
    field.components = {"value"};
    frame.fields.push_back(field);
  }
  check(CoreResultsUnits::annotate(results, error, {{1, "frequency"}}), "Annotate explicit frequency axis and physical fields");
  check(frame.independent_variable.quantity == "frequency", "mode=-1 does not imply time");
  const std::string expected[] = {"heat_flux", "power", "energy_density", "strain"};
  for (std::size_t index = 0; index < frame.fields.size(); ++index)
  {
    check(frame.fields[index].component_units.size() == 1 && frame.fields[index].component_units[0].quantity == expected[index], "Result component has its own physical quantity");
  }
  check(CoreResultsUnits::annotate(results, error) && frame.independent_variable.quantity.empty(), "Missing axis provenance stays unknown");
}
void provenance(const QString& directory)
{
  std::string error;
  const auto inputPath = directory + "/units.inp";
  QFile input(inputPath);
  check(input.open(QIODevice::WriteOnly), "Open metadata fixture");
  input.write("** CUBIT_SIM_UNIT_SYSTEM=N-mm-s\n** CUBIT_SIM_GEOMETRY_UNIT=mm\n** CUBIT_SIM_STEP_AXIS=1;frequency\n");
  input.close();
  CalculiXJobRunRecord record;
  check(record.read_input_units(inputPath.toStdString(), error) && record.input.files.empty(), "Topology metadata does not retain a complete input archive");
  check(record.step_axis_quantities.count(1) == 1 && record.step_axis_quantities.at(1) == "frequency", "Read emitted solver ordinal independently of model step ID");
  CoreResultsSourceMetadata metadata(record);
  CoreResultsSourceMetadata restored;
  check(CoreResultsSourceMetadata::from_json(metadata.to_json(), restored, error) && restored.step_axis_quantities == record.step_axis_quantities, "Result axis provenance survives serialization");
  record.jobId = 1;
  record.launchId = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
  record.launchedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toStdString();
  record.setupUnavailableReason = "Metadata-only regression fixture.";
  const auto save = (directory + "/record.hdf5").toStdString();
  {
    HDF5Tool file(save);
    check(CoreJobRunPersistence::write(file, {{1, std::make_shared<const CalculiXJobRunRecord>(record)}}, error), "Save metadata-only run record");
  }
  {
    HDF5Tool file(save);
    std::map<int, std::shared_ptr<const CalculiXJobRunRecord>> records;
    const bool read = CoreJobRunPersistence::read(file, records, error);
    check(read && records.count(1) == 1 && records.at(1)->step_axis_quantities == record.step_axis_quantities && records.at(1)->unitSystem == "N-mm-s", "Metadata-only run provenance survives HDF5 reopen");
  }
}
}

int main(int argc, char** argv)
{
  QCoreApplication application(argc, argv);
  QTemporaryDir directory;
  if (!directory.isValid())
  {
    return 1;
  }
  dofs();
  geometry_units();
  solver_number_format();
  axes_and_fields();
  provenance(directory.path());
  return failures;
}
