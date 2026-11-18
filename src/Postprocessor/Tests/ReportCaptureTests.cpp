#include "ReportCaptureTests.hpp"
#include "Postprocessor/Interfaces/PostprocessorReportCapturePlan.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkNew.h>
#include <vtkCellType.h>
#include <cmath>
#include <iostream>

int report_capture_cases()
{
  using namespace Postprocessor;
  int failures = 0;
  const auto check = [&](bool condition, const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  SimUnits::WorkspaceUnitContext reportUnits;
  check(framework_units().workspace_context("report", reportUnits, error), "Cannot retain Report units");
  check(framework_units().set_display_unit("report", "stress", "Pa", error), "Cannot set Report test units");
  auto data = std::make_shared<SimResults::VtkResultsData>();
  auto mesh = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(), [](auto* value)
  {
    value->Delete();
  });
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0, 0, 0);
  mesh->SetPoints(points);
  const vtkIdType vertex = 0;
  mesh->InsertNextCell(VTK_VERTEX, 1, &vertex);
  SimResults::Dataset block;
  block.id = 0;
  block.name = "Block 1";
  block.kind = "Blocks";
  block.model_id = 1;
  data->datasets.push_back(block);
  SimResults::Field stress;
  stress.name = "Stress";
  stress.array_name = "stress";
  stress.components = {"XX"};
  stress.component_units = {SimUnits::QuantityUnit("stress", "MPa", SimUnits::Stress)};
  SimResults::Field displacement = stress;
  displacement.name = "Displacement";
  displacement.array_name = "displacement";
  displacement.component_units = {SimUnits::QuantityUnit("length", "mm", SimUnits::Length)};
  SimResults::Increment increment;
  increment.dataset = mesh;
  increment.fields = {stress, displacement};
  data->increments = {increment, increment};
  FieldSelection selection;
  selection.dataset_id = 0;
  selection.array_name = stress.array_name;
  selection.component = "XX";
  CaptureSnapshot snapshot;
  snapshot.target_id = "view1";
  snapshot.sources["source"] = data;
  snapshot.session.sources["source"] = DataSource{"source", "Source", {}};
  auto& view = snapshot.session.views.at("view1");
  view.camera = CameraDefinition{};
  view.camera->position = {4, 5, 6};
  view.scene_unit = "m";
  snapshot.options.backgroundTop = {0.1, 0.2, 0.3};
  snapshot.reference_size = {900, 700};
  FilterDefinition clip;
  clip.id = "clip";
  clip.input.object_id = "source";
  clip.coordinate_unit = "mm";
  snapshot.session.filters[clip.id] = clip;
  FilterDefinition threshold;
  threshold.id = "threshold";
  threshold.kind = FilterKind::Threshold;
  threshold.input = {OutputOwner::Feature, clip.id, 0};
  threshold.threshold.field = selection;
  threshold.threshold.lower = 2.0;
  threshold.threshold.bounds_unit = stress.component_units.front();
  snapshot.session.filters[threshold.id] = threshold;
  auto unrelated = clip;
  unrelated.id = "unrelated";
  snapshot.session.filters[unrelated.id] = unrelated;
  Representation display;
  display.id = "display";
  display.input = {OutputOwner::Feature, threshold.id, 0};
  display.view_id = "view1";
  display.coloring.field = selection;
  display.coloring.palette = ColorPalette::Viridis;
  display.coloring.range_mode = ColorRangeMode::Fixed;
  display.coloring.fixed_range = {2, 8};
  display.coloring.bounds_unit = stress.component_units.front();
  display.vectors.enabled = true;
  display.vectors.scale = 3.5;
  display.vectors.maximum = 123;
  display.deformation.enabled = true;
  display.deformation.scale = 2.5;
  display.dataset_appearance[0].style = DisplayStyle::Wireframe;
  display.dataset_appearance[0].solid_color = std::array<double, 3>{0.8, 0.2, 0.1};
  display.hidden_datasets.insert(0);
  snapshot.session.representations[display.id] = display;
  snapshot.session.active_representation_id = display.id;
  ReportConfiguration settings;
  PostprocessorReportCapturePlan plan;
  check(plan.initialize(snapshot, "source", settings, error), "Cannot freeze inherited presentation");
  // Live unit changes must not change a collection that has already frozen.
  check(framework_units().set_display_unit("report", "stress", "MPa", error), "Cannot change live test units");
  CaptureSnapshot sample;
  check(plan.prepare(1, 0, selection, sample, error), "Cannot prepare inherited report frame");
  if (sample.session.representations.count(display.id) != 0)
  {
    const auto& result = sample.session.representations.at(display.id);
    check(result.input.object_id == threshold.id && sample.session.filters.size() == 2,
      "Report lost its upstream chain or retained unrelated filters");
    check(sample.session.filters.at(threshold.id).threshold.lower == 2.0 &&
      sample.session.filters.at(threshold.id).threshold.bounds_unit->unit == "MPa",
      "Report reinterpreted the inherited threshold units");
    check(result.increment_index == 1 && result.hidden_datasets.count(0) == 0,
      "Report did not override frame and explicit block visibility");
    check(result.dataset_appearance.at(0).style == DisplayStyle::Wireframe &&
      result.dataset_appearance.at(0).solid_color == display.dataset_appearance.at(0).solid_color &&
      result.vectors.scale == 3.5 && result.vectors.maximum == 123 && result.deformation.scale == 2.5,
      "Report lost per-item style, color, vectors or deformation");
    check(result.coloring.palette == ColorPalette::Viridis &&
      std::abs(result.coloring.fixed_range[0] - 2000000.0) < 0.01 &&
      result.coloring.bounds_unit->unit == "Pa", "Report failed to convert and freeze scalar bounds");
    check(sample.session.views.at("view1").camera->position == view.camera->position &&
      sample.session.views.at("view1").scene_unit == "m" && sample.reference_size == snapshot.reference_size &&
      sample.options.backgroundTop == snapshot.options.backgroundTop, "Report changed camera, scene basis or background");
  }
  selection.array_name = displacement.array_name;
  check(plan.prepare(0, 0, selection, sample, error) &&
    sample.provenance.value("report_capture").toObject().value("automatic_range_adjustment").toBool(),
    "Incompatible limits were not replaced and disclosed");
  const auto previous = sample.provenance;
  check(!plan.prepare(99, 0, selection, sample, error) && sample.provenance == previous,
    "Failed preparation modified the previous output");
  check(snapshot.session.representations.at(display.id).hidden_datasets.count(0) != 0 &&
    snapshot.session.representations.at(display.id).coloring.fixed_range[0] == 2,
    "Report mutated its input presentation");
  auto ambiguous = snapshot;
  ambiguous.session.active_representation_id.clear();
  auto second = display;
  second.id = "second";
  ambiguous.session.representations[second.id] = second;
  check(!plan.initialize(ambiguous, "source", settings, error), "Ambiguous presentation was chosen arbitrarily");
  auto broken = snapshot;
  broken.session.filters.at("clip").input = {OutputOwner::Feature, "threshold", 0};
  check(!plan.initialize(broken, "source", settings, error), "Cyclic report filter graph was accepted");
  settings.values["post.images.camera"] = "fit";
  settings.values["post.images.background"] = "white";
  check(plan.initialize(snapshot, "source", settings, error) && plan.prepare(0, 0, selection, sample, error) &&
    !sample.session.views.at("view1").camera && sample.options.backgroundTop == std::array<double, 3>{1, 1, 1},
    "Explicit fit/white overrides did not apply");
  check(framework_units().set_workspace_context(reportUnits, error), "Cannot restore Report test units");
  return failures;
}
