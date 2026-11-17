#include "ReportTestProviders.hpp"
#include "Framework/Storage/ReportProfileStore.hpp"
#include "Framework/Storage/UserOptionsStore.hpp"
#include <QJsonArray>
#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportUnitValidation.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Document/ReportFigureCaption.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Document/ReportOdtExporter.hpp"
#include "Report/Document/ReportPdfExporter.hpp"
#include "CalculiX/Core/reporting/CalculiXReportInputReader.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include <QFile>
#include <cmath>
#include <iostream>

int reportUnitsTests(const QString& directory)
{
  int failures = 0;
  const auto check = [&](bool result, const char* message)
  {
    if (!result)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  FrameworkPaths paths;
  paths.optionsFilename = (directory + "/report-unit-options.h5").toStdString();
  UserOptionsStore options(paths);
  UserOptionValues optionValues;
  check(options.load("test", {{"sentinel", UserOptionKind::Integer, 1}}, optionValues, error), "Initialize unit test profile store");
  ReportProfileStore profiles(paths.optionsFilename);
  SimUnits::WorkspaceUnitContext original;
  if (!framework_units().workspace_context("report", original, error))
  {
    check(false, "Read original Report unit context");
    return failures;
  }
  check(framework_units().set_workspace_system("report", "SI", error), "Choose SI Report units");
  check(framework_units().set_display_unit("report", "length", "mm", error), "Override Report length");
  SimUnits::WorkspaceUnitContext selected;
  check(framework_units().workspace_context("report", selected, error), "Capture selected units");
  const auto savedUnits = ReportUnitSettings::capture(selected);

  ReportTestProviders providers;
  providers.profiles = &profiles;
  ReportCore core(providers);
  check(core.generate("test", 1, "", error), "Generate a report with resolved unit preferences");
  std::vector<ReportProfile> saved;
  check(core.profile("save", "Unit regression", saved, error), "Save profile with Report units");
  ReportProfile stored;
  check(profiles.load("test", "Unit regression", stored, error) && stored.configuration.units &&
    stored.configuration.units->to_json() == savedUnits.to_json(), "Profile persists presentation preferences");

  check(framework_units().set_workspace_system("report", "N-mm-ms", error), "Switch units before profile load");
  core.units_changed();
  const auto immutable = core.document();
  check(core.profile("load", "Unit regression", saved, error), "Load profile unit preferences");
  SimUnits::WorkspaceUnitContext restored;
  check(framework_units().workspace_context("report", restored, error) &&
    ReportUnitSettings::capture(restored).to_json() == savedUnits.to_json(), "Profile restores units and overrides");
  check(core.document() == immutable && core.session().configuration_changed, "Profile load keeps the old document immutable");
  check(core.update(error) && core.document()->configuration.units &&
    core.document()->configuration.units->to_json() == savedUnits.to_json(), "Update records actual generation units");

  auto invalid = stored;
  invalid.name = "Invalid unit regression";
  invalid.configuration.units->display_units["length"] = "s";
  check(profiles.save(invalid, error), "Persist structurally valid but physically invalid profile fixture");
  const auto beforeInvalid = core.document();
  const auto beforeRequest = core.configuration_request().to_json();
  check(!core.profile("load", invalid.name, saved, error) && core.document() == beforeInvalid &&
    core.configuration_request().to_json() == beforeRequest, "Invalid profile is rejected without partial configuration changes");
  check(framework_units().workspace_context("report", restored, error) &&
    ReportUnitSettings::capture(restored).to_json() == savedUnits.to_json(), "Invalid profile does not change preferences");
  check(core.generate("test", 2, "", error), "Generate a second source");
  check(framework_units().set_display_unit("report", "length", "m", error), "Change units for all reports");
  core.units_changed();
  check(core.session().units_pending_update.size() == 2, "Unit changes mark both generated sources for update");
  ReportSession roundtrip;
  check(ReportSession::from_json(core.session().to_json(), roundtrip, error) &&
    roundtrip.units_pending_update == core.session().units_pending_update, "Pending unit updates survive session serialization");
  ReportRequest first;
  first.provider = "test";
  first.job_id = 1;
  check(core.choose_source(first, error) && core.session().configuration_changed, "Switching sources retains unit update requirement");
  check(core.update(error) && core.session().units_pending_update.size() == 1, "Updating clears only that source's unit requirement");

  ReportApplicationInterface exportApplication(providers);
  check(exportApplication.generate("test", 1, "", error), "Generate report for unit export guard");
  check(exportApplication.configure_units("", "length", "cm", error), "Change units through the application command interface");
  check(!exportApplication.export_document("pdf", (directory + "/stale-unit-report.pdf").toStdString(), error),
    "Export is blocked until changed Report units are applied by Update");
  check(exportApplication.update(error), "Update restores export readiness after unit changes");

  ReportDocument typed = *core.document();
  ReportTable table;
  table.title = "Typed temperature values";
  table.columns = {"Temperature", "Temperature difference"};
  table.units = {"degC", "delta_degC"};
  SimUnits::QuantityUnit absolute, difference;
  check(framework_units().quantity("temperature", absolute, error) &&
    framework_units().quantity("temperature_difference", difference, error), "Read temperature roles");
  absolute.unit = "degC";
  difference.unit = "delta_degC";
  table.column_quantities = {absolute, difference};
  table.rows = {{20.0, 5.0}};
  typed.sections.front().tables.push_back(table);
  check(ReportUnitValidation::document(typed, error), "Absolute temperatures and differences validate separately");
  ReportDocument decoded;
  check(ReportDocument::from_json(typed.to_json(), decoded, error) && ReportUnitValidation::document(decoded, error),
    "Typed quantities survive report serialization");
  typed.sections.front().tables.back().column_quantities[0]->role = SimUnits::UnitRole::TemperatureDifference;
  check(!ReportUnitValidation::document(typed, error), "Reject incorrect physical temperature roles");
  typed = decoded;
  typed.sections.front().tables.back().column_quantities[0]->dimension = SimUnits::Length;
  check(!ReportUnitValidation::document(typed, error), "Reject unit dimension conflicts");
  typed = decoded;
  typed.sections.front().tables.back().cell_quantities = {{}};
  check(!ReportUnitValidation::document(typed, error), "Reject malformed per-cell quantity shapes");
  typed = decoded;
  typed.sections.front().tables.back().units[0] = "unknown unit";
  typed.sections.front().tables.back().column_quantities[0].reset();
  check(ReportUnitValidation::document(typed, error), "Explicit unknown units remain unconverted");

  ReportImage image;
  image.caption = "Displacement overview";
  image.provenance.insert("presentation_units", savedUnits.to_json());
  ReportConfiguration configuration;
  SimUnits::WorkspaceUnitContext current;
  check(framework_units().workspace_context("report", current, error), "Read different Report units for figure comparison");
  configuration.units = ReportUnitSettings::capture(current);
  const auto caption = ReportFigureCaption::text(image, configuration);
  check(caption.contains("Capture preferences: SI") && caption.contains("length: mm") &&
    caption.contains("retains its capture units"), "Figure captions identify captured preferences independently of table units");

  const auto deck = directory + "/report-unit-source.inp";
  QFile input(deck);
  check(input.open(QIODevice::WriteOnly), "Open input unit fixture");
  input.write("** CUBIT_SIM_UNIT_SYSTEM=N-mm-s\n** CUBIT_SIM_GEOMETRY_UNIT=m\n*NODE\n1,1000,0,0\n");
  input.close();
  ReportSource source;
  source.workspace_id = "calculix";
  source.job_id = 1;
  source.input_files = {deck.toStdString()};
  CalculiXReportInputReader reader;
  check(reader.read(source, error) && reader.resolved_source().source_units &&
    reader.resolved_source().source_units->coordinate_unit == "mm", "Deck coordinate units are distinct from native geometry units");
  check(reader.records().first().toObject().value("data").toArray().first().toString().contains("1000"),
    "Source input numeric text remains unchanged");
  check(input.open(QIODevice::WriteOnly | QIODevice::Truncate), "Replace unit fixture");
  input.write("*NODE\n1,1000,0,0\n");
  input.close();
  check(reader.read(source, error) && !reader.resolved_source().source_units, "Missing declarations remain unknown despite active Report units");

  ReportDocumentLayout layout;
  check(layout.prepare(decoded, error), "Build preview layout from stored units");
  const auto frozen = decoded.to_json();
  check(ReportPdfExporter().write(layout, (directory + "/unit-regression.pdf").toStdString(), error), "Export PDF with stored unit values");
  check(ReportOdtExporter().write(layout.report(), (directory + "/unit-regression.odt").toStdString(), error), "Export ODT with stored unit values");
  QFile odt(directory + "/unit-regression.odt");
  check(odt.open(QIODevice::ReadOnly), "Read exported unit labels");
  const auto odtBytes = odt.readAll();
  check(odtBytes.contains("Temperature [degC]") && odtBytes.contains("Temperature difference [delta_degC]"),
    "ODT preserves the preview document's absolute and difference unit labels");
  check(decoded.to_json() == frozen, "Preview and exporters do not mutate stored physical values");

  check(framework_units().set_workspace_context(original, error), "Restore original Report preferences");
  return failures;
}
