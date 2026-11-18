#include "ReportMeshQualityTests.hpp"
#include "Framework/MeshQuality/MeshQualityDefaults.hpp"
#include "Framework/MeshQuality/MeshQualitySettings.hpp"
#include "Framework/MeshQuality/MeshQualityStatistics.hpp"
#include "Framework/MeshQuality/MeshQualityPresentation.hpp"
#include "Framework/MeshQuality/MeshQualitySnapshotCodec.hpp"
#include "Framework/Reporting/ReportMeshQualityContent.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Report/GUI/ReportMeshQualitySettings.hpp"
#include "Report/GUI/ReportSettingsCommandText.hpp"
#include "ReportTestProgress.hpp"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <cmath>
#include <iostream>
#include <limits>

int reportMeshQualityTests()
{
  int failures = 0;
  const auto check = [&](bool condition, const char* message)
  {
    if (!condition)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  };
  ReportTemplate definition;
  definition.content_groups.push_back({"ccx.content.model", "Model"});
  MeshQualityDefaults::add_options(definition);
  ReportConfigurationResolver::add_common_options(definition);
  ReportConfiguration configuration;
  std::string error;
  check(ReportConfigurationResolver::resolve(definition, {}, configuration, error), "Quality defaults do not validate");
  MeshQualitySettings settings;
  check(MeshQualitySettings::decode(configuration, settings, error) && settings.metrics.size() == 4 &&
    !settings.element_values && settings.statistics && settings.images, "Unexpected quality inclusion defaults");
  check(configuration.values.at("mesh.quality.hex.scaled_jacobian.maximum") == "1.01",
    "Hex quality screen's upper tolerance was not preserved");
  auto invalid = configuration;
  invalid.values["mesh.quality.hex.scaled_jacobian.minimum"] = "2";
  const auto original = settings.metrics.size();
  check(!MeshQualitySettings::decode(invalid, settings, error) && settings.metrics.size() == original,
    "Inverted limits changed the previous settings");
  invalid.values["mesh.quality.hex.scaled_jacobian.minimum"] = "nan";
  check(!ReportConfigurationResolver::resolve(definition, invalid, configuration, error), "Nonfinite quality bounds were accepted");
  check(ReportConfigurationResolver::resolve(definition, {}, configuration, error), "Cannot restore test defaults");
  ReportMeshQualitySettings page(configuration, nullptr);
  auto* family = page.findChild<QComboBox*>("ReportQualityFamily");
  check(family && family->count() == 4, "Missing family sibling pages");
  if (family)
  {
    family->setCurrentIndex(1);
  }
  auto* minimum = page.findChild<QLineEdit*>("mesh.quality.tet.scaled_jacobian.minimum");
  auto* restore = page.findChild<QPushButton*>("ReportQualityRestore");
  check(minimum && restore, "Missing editable quality limits or reset control");
  if (minimum && restore)
  {
    minimum->setText("0.35");
    ReportConfiguration patch;
    check(page.configuration_patch(patch, error) && patch.values.at("mesh.quality.tet.scaled_jacobian.minimum") == "0.35",
      "Family page did not retain an edited limit");
    std::string command;
    ReportConfiguration edited;
    edited.values["mesh.quality.tet.scaled_jacobian.minimum"] = "0.35";
    check(ReportSettingsCommandText::build(edited, {}, command, error, &definition) &&
      command.find("option \"mesh.quality.tet.scaled_jacobian.minimum\" value \"0.35\"") != std::string::npos,
      "Quality edit did not produce a human-readable command");
    restore->click();
    check(page.configuration_patch(patch, error) && QString::fromStdString(patch.values.at("mesh.quality.tet.scaled_jacobian.minimum")).toDouble() == .2,
      "Family reset did not use the Core defaults");
  }
  MeshQualityMetric metric;
  metric.family = "tet";
  metric.id = "scaled_jacobian";
  metric.minimum = .2;
  metric.maximum = 1;
  MeshQualityMetric volume;
  volume.quantity = "volume";
  volume.unit = "mm^3";
  volume.minimum = 0;
  volume.maximum = 8;
  ReportConfiguration unitConfiguration;
  unitConfiguration.units = ReportUnitSettings();
  unitConfiguration.units->system = "N-mm-s";
  unitConfiguration.units->display_units["volume"] = "m^3";
  MeshQualityPresentation presentation;
  const std::vector<double> nativeVolumes = {1, 8};
  check(presentation.prepare(volume, "mm", unitConfiguration, nativeVolumes, nullptr,
      ReportProgressStage::Model, error) && presentation.metric.unit == "m^3" &&
    std::abs(presentation.values(nativeVolumes)[0] - 1e-9) < 1e-20 &&
    std::abs(*presentation.metric.maximum - 8e-9) < 1e-20,
    "Dimensional quality values and acceptance limits did not convert together");
  check(!presentation.prepare(volume, "", unitConfiguration, nativeVolumes, nullptr,
      ReportProgressStage::Model, error), "Unknown geometry units were guessed for dimensional quality");
  MeshQualityStatistics statistics;
  check(statistics.calculate({9, 2}, {1, 3}, metric, error) && statistics.mean == 2 &&
    statistics.standard_deviation == 1 && statistics.failed == 1, "Population quality statistics are incorrect");
  check(statistics.calculate({9, 2, 3}, {.1, .1, std::numeric_limits<double>::quiet_NaN()}, metric, error) &&
    statistics.minimum_id == 2 && statistics.maximum_id == 2 && statistics.unevaluable == 1 && statistics.failed == 2,
    "Quality invalid-sample accounting or extrema tie handling is incorrect");
  MeshQualitySnapshot snapshot;
  snapshot.coordinate_unit = "mm";
  snapshot.input_fingerprint = "test-input";
  snapshot.node_ids = {1, 2, 3, 4};
  snapshot.coordinates = {{{0,0,0}}, {{1,0,0}}, {{0,1,0}}, {{0,0,1}}};
  MeshQualityBlock block;
  block.block_id = 1;
  block.block_name = "Block_1";
  block.family = "tet";
  block.nodes_per_element = 4;
  block.element_ids = {10, 20};
  block.connectivity = {1, 2, 3, 4, 1, 2, 3, 4};
  block.values["scaled_jacobian"] = {.1, .9};
  snapshot.blocks = {block};
  snapshot.mesh_fingerprint = snapshot.fingerprint();
  check(snapshot.validate(error), "Quality fixture has invalid topology");
  QByteArray bytes;
  std::shared_ptr<const MeshQualitySnapshot> decoded;
  check(MeshQualitySnapshotCodec::encode(snapshot, bytes, error) &&
    MeshQualitySnapshotCodec::decode(bytes, decoded, error) && decoded->mesh_fingerprint == snapshot.mesh_fingerprint &&
    decoded->blocks.front().values.at("scaled_jacobian") == block.values.at("scaled_jacobian"),
    "Quality archive did not round-trip identities and values");
  if (!bytes.isEmpty())
  {
    bytes[0] = char(bytes[0] ^ 1);
    const auto before = decoded;
    check(!MeshQualitySnapshotCodec::decode(bytes, decoded, error) && decoded == before,
      "Corrupt quality archive replaced the previous snapshot");
  }
  check(MeshQualitySettings::decode(configuration, settings, error), "Cannot restore quality test settings");
  ReportDocument report;
  check(ReportMeshQualityContent::append(snapshot, settings, nullptr, report, error) &&
    report.sections.size() == 1 && report.sections[0].children[0].tables.size() == 1,
    "Detailed element tables were generated by default");
  settings.element_values = true;
  ReportDocument detailed;
  check(ReportMeshQualityContent::append(snapshot, settings, nullptr, detailed, error) &&
    detailed.sections[0].children[0].tables.size() == 2, "Explicit element listing was omitted");
  ReportTestProgress cancel;
  cancel.cancelStage = "Summarizing quality";
  const auto previousSections = report.sections.size();
  check(!ReportMeshQualityContent::append(snapshot, settings, &cancel, report, error) &&
    report.sections.size() == previousSections, "Quality cancellation published partial content");
  return failures;
}
