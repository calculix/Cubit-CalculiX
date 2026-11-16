#include "Core/jobs/CoreJobSnapshotBuilder.hpp"
#include "Core/jobs/CoreJobRunPersistence.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/drawing/CoreDrawSnapshotCapture.hpp"
#include "Core/reporting/CalculiXReportInputReader.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <cmath>
#include <QDateTime>
#include <QFile>
#include <QTemporaryDir>
#include <iostream>

int jobSnapshotTests()
{
  int failures = 0;
  const auto check = [&](bool value, const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  QTemporaryDir directory;
  std::string error;
  const auto root = directory.filePath("model.inp");
  const auto mesh = directory.filePath("mesh input.inp");
  const auto write = [&](const QString& path, const QByteArray& bytes)
  {
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
  };
  check(write(mesh, "*NODE\n1,0,0,0\n2,1,0,0\n3,0,1,0\n") &&
      write(root, "** CUBIT_SIM_UNIT_SYSTEM=N-mm-s\n** CUBIT_SIM_GEOMETRY_UNIT=mm\n*INCLUDE, INPUT=\"mesh input.inp\"\n*STEP\n*STATIC\n*END STEP\n"), "Create archived input fixture");
  CalculiXJobRunRecord record;
  record.jobId = 1;
  record.launchId = "launch-one";
  record.launchedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toStdString();
  check(CoreJobSnapshotBuilder::captureInput(root.toStdString(), record.input, error) && record.input.files.size() == 2,
    "Archive exact input bytes and quoted INCLUDE dependencies");
  check(record.read_input_units(error), "Read snapshot coordinate provenance from the archived deck");
  {
    CalculiXJobRunRecord meshRecord;
    meshRecord.input.rootFile = "mesh.inp";
    meshRecord.input.files["mesh.inp"] = "*NODE\n1,0,0,0\n2,0.1,0,0\n3,0,0.1,0\n*ELEMENT, TYPE=S3, ELSET=BLOCK_1\n1,1,2,3\n*STEP\n*STATIC\n*END STEP\n";
    const std::map<std::string, std::string> colors = {{"BLOCK_1", "#123456"}};
    DrawingGeometry geometry;
    double size = -1;
    int steps = -1;
    for (const auto& units : {std::pair<const char*, const char*>{"SI", "mm"}, {"N-mm-s", "m"}, {"N-mm-s", "mm"}})
    {
      meshRecord.unitSystem = units.first;
      meshRecord.geometryLengthUnit = units.second;
      double expected = 0.1;
      if (meshRecord.unitSystem == "SI")
      {
        expected = 100.0;
      }
      else if (meshRecord.geometryLengthUnit == "m")
      {
        expected = 0.0001;
      }
      const bool built = CoreJobSnapshotBuilder::buildSetupModel(meshRecord, colors, geometry, size, steps, error);
      check(built, "Build exported mesh in its recorded geometry units");
      if (built)
      {
        check(steps == 1 && geometry.primitives.size() == 1 &&
            std::abs(geometry.primitives.front().points[1][0] - expected) < 1e-12 &&
            std::abs(size - expected * 0.1) < 1e-12,
          "Mesh positions and symbol size share the camera/condition coordinate basis");
      }
    }
    const double previousSize = size;
    const int previousSteps = steps;
    const auto previousCount = geometry.primitives.size();
    meshRecord.geometryLengthUnit.clear();
    check(!CoreJobSnapshotBuilder::buildSetupModel(meshRecord, colors, geometry, size, steps, error) &&
        size == previousSize && steps == previousSteps && geometry.primitives.size() == previousCount,
      "Unknown coordinate units preserve the previous snapshot outputs");
    meshRecord.unitSystem = "SI";
    meshRecord.geometryLengthUnit = "mm";
    meshRecord.input.files["mesh.inp"].replace("0.1,0,0", "1e308,0,0");
    check(!CoreJobSnapshotBuilder::buildSetupModel(meshRecord, colors, geometry, size, steps, error) &&
        size == previousSize && steps == previousSteps && geometry.primitives.size() == previousCount,
      "Overflow in coordinate scaling preserves previous snapshot outputs");
  }
  auto setup = std::make_shared<CalculiXJobSetupSnapshot>();
  setup->camera.available = true;
  setup->camera.position = {2, -3, 4};
  setup->camera.up = {0, 0, 1};
  setup->camera.parallelScale = 8;
  setup->id = "snapshot-one";
  setup->symbolSize = 0.1;
  setup->model.polygon({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}, "#123456");
  setup->modelDefinitions.unavailableReason = "Job conditions require a selected step.";
  CalculiXJobSetupScene scene;
  scene.available = true;
  scene.stepId = 7;
  scene.ordinal = 1;
  scene.name = "Frozen step";
  const DrawEntityKey force = {DrawFeature::Force, 3};
  scene.symbols[force].line({0, 0, 0}, {0, 0.1, 0}, "green");
  DrawLabelData label;
  label.entity = force;
  label.stepId = 7;
  label.symbolSize = 0.1;
  label.name = "Original force";
  label.typeName = "Force";
  label.values = {{"FX", "100"}};
  label.anchors = {{{0, 0, 0}}};
  scene.labels[force] = label;
  setup->steps.push_back(scene);
  setup->fingerprint = CoreJobSnapshotBuilder::setupFingerprint(*setup);
  record.setup = setup;
  check(CoreJobSnapshotBuilder::validate(record, error), "Validate complete launch/input/setup linkage");
  auto missingUnits = record;
  missingUnits.geometryLengthUnit.clear();
  check(!CoreJobSnapshotBuilder::validate(missingUnits, error), "A typed setup cannot omit coordinate provenance");
  const auto archived = record.input;
  check(write(mesh, "*NODE\n1,99,99,99\n"), "Change external input after launch");
  ReportSource source;
  source.workspace_id = "calculix";
  source.job_id = 1;
  source.solver_launch_id = record.launchId;
  source.setup_snapshot_id = setup->id;
  source.input_files = archived.order;
  CalculiXReportInputReader reader;
  check(QFile::remove(root) && QFile::remove(mesh) && reader.read_archive(source, archived, error) && reader.records().size() == 4,
    "Archived report input remains readable after external files are changed and deleted");
  auto corrupt = record;
  corrupt.input.files.begin()->second += "tampered";
  check(!CoreJobSnapshotBuilder::validate(corrupt, error), "Reject mismatched archived input bytes");
  CalculiXCaptureRequest request;
  request.scope = {DrawScopeKind::Step, 7};
  request.labels.showValues = true;
  DrawingFrame frame;
  check(CoreDrawSnapshotCapture::buildFrame(*record.setup, request, frame, error) && frame.entities.size() == 2 &&
      frame.entities[1].geometry.primitives.back().text.find("Original force") != std::string::npos &&
      frame.shadedModelMesh && frame.camera.toJson() == setup->camera.toJson(),
    "Compose historical geometry and labels without querying a live model");
  auto changed = *record.setup;
  changed.steps.front().labels[force].name = "Changed current definition";
  check(CoreDrawSnapshotCapture::buildFrame(*record.setup, request, frame, error) &&
      frame.entities[1].geometry.primitives.back().text.find("Original force") != std::string::npos,
    "Changes to a separate model copy cannot affect the frozen report scene");
  request.labels.enabled = false;
  check(CoreDrawSnapshotCapture::buildFrame(*record.setup, request, frame, error) && frame.entities[1].geometry.primitives.size() == 1,
    "Historical image profile changes remove labels without changing stored symbols");
  request.scope.stepId = 9;
  check(!CoreDrawSnapshotCapture::buildFrame(*record.setup, request, frame, error) && frame.entities.size() == 2,
    "Missing frozen steps preserve the previous output frame");
  std::map<int, std::shared_ptr<const CalculiXJobRunRecord>> records = {{1, std::make_shared<const CalculiXJobRunRecord>(record)}};
  const auto project = directory.filePath("snapshots.cub").toStdString();
  {
    HDF5Tool file(project, true);
    check(CoreJobRunPersistence::write(file, records, error) && file.close(), "Persist input bytes and structured setup geometry");
  }
  std::map<int, std::shared_ptr<const CalculiXJobRunRecord>> restored;
  {
    HDF5Tool file(project, false);
    check(CoreJobRunPersistence::read(file, restored, error) && restored.size() == 1 && restored.at(1)->setup &&
        restored.at(1)->setup->fingerprint == setup->fingerprint && restored.at(1)->input.files == archived.files &&
        restored.at(1)->setup->camera.toJson() == setup->camera.toJson() &&
        restored.at(1)->setup->model.primitives.front().color == "#123456",
      "Round-trip archived input, labels and geometry without reading the external model");
    // Corrupt a bounded numeric payload; restore must fail atomically.
    const auto groups = file.getGroupsFromFile();
    std::string modelPath;
    for (const auto& group : groups)
    {
      if (group.size() >= 12 && group.substr(group.size() - 12) == "/Setup/Model")
      {
        modelPath = group;
      }
    }
    if (!modelPath.empty())
    {
      check(file.write_dataset_double_rank_1("points", modelPath, {0, 0, 0}), "Corrupt snapshot fixture geometry");
      const auto previous = restored.at(1);
      check(!CoreJobRunPersistence::read(file, restored, error) && restored.at(1) == previous,
        "Corrupt HDF5 snapshot data cannot partially replace the loaded run archive");
    }
    else
    {
      check(false, "Find persisted model geometry group");
    }
  }
  CoreJobs jobs;
  jobs.publish_run_record(CalculiXJobRunRecord(record));
  std::shared_ptr<const CalculiXJobRunRecord> published;
  check(jobs.get_run_record(1, published, error) && published->launchId == "launch-one", "Publish immutable record after simulated launch success");
  // Preparing another launch alone must leave the published launch untouched.
  CalculiXJobRunRecord pending;
  pending.jobId = 1;
  pending.launchId = "launch-two";
  pending.setupUnavailableReason = "Imported input has no typed setup.";
  check(jobs.get_run_record(1, published, error) && published->setup, "A pending or failed launch preserves the previous launched setup");
  jobs.publish_run_record(std::move(pending));
  check(jobs.get_run_record(1, published, error) && published->launchId == "launch-two" && !published->setup,
    "A newer unverified launch cannot inherit the old run's setup snapshot");
  return failures;
}
