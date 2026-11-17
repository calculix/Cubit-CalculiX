#include "Postprocessor/Interfaces/PostprocessorReportFrameSelection.hpp"
#include "Postprocessor/Interfaces/PostprocessorReportExtrema.hpp"
#include "Postprocessor/Interfaces/PostprocessorReportBlockSummary.hpp"
#include <vtkNew.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellType.h>
#include <QCoreApplication>
#include <iostream>
#include <limits>

int main(int argc, char** argv)
{
  QCoreApplication application(argc, argv);
  int failures = 0;
  const auto check = [&](bool condition, const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  SimResults::VtkResultsData source;
  for (int step : {1, 1, 2, 2})
  {
    SimResults::Increment frame;
    frame.step = step;
    frame.time = 100.0 - source.increments.size();
    source.increments.push_back(frame);
  }
  ReportConfiguration settings;
  settings.values["steps"] = "all";
  settings.values["post.results.frames"] = "last_per_step";
  std::vector<std::size_t> frames;
  std::string error;
  PostprocessorReportFrameSelection selection;
  check(selection.select(source, settings, frames, error) && frames == std::vector<std::size_t>({1, 3}),
    "Last frame selection uses canonical order, independently of decreasing time");
  settings.values["steps"] = "2";
  check(selection.select(source, settings, frames, error) && frames == std::vector<std::size_t>({3}),
    "Frame selection respects selected step ordinals");
  settings.values["steps"] = "all";
  for (const std::string mode : {"envelope", "all_frames"})
  {
    settings.values["post.results.frames"] = mode;
    check(selection.select(source, settings, frames, error) && frames.size() == 4,
      "Envelope and every-frame policies retain selected frames");
  }
  source.increments.front().step = -1;
  const auto before = frames;
  check(!selection.select(source, settings, frames, error) && frames == before,
    "Invalid frame identity leaves output unchanged");
  source.increments.front().step = 1;
  std::atomic<bool> stop{false};
  PostprocessorReportExtrema first, second;
  first.accumulate({3.0, -2.0, std::numeric_limits<double>::quiet_NaN()}, 0, stop);
  second.accumulate({-2.0, 8.0, 8.0}, 3, stop);
  auto reverse = second;
  reverse.merge(first);
  first.merge(second);
  check(first.available && first.minimum == -2.0 && first.maximum == 8.0 &&
    first.minimum_sample == 1 && first.maximum_sample == 4 &&
    reverse.minimum_sample == first.minimum_sample && reverse.maximum_sample == first.maximum_sample,
    "Finite extrema and tie locations are independent of merge order");
  vtkNew<vtkUnstructuredGrid> mesh;
  vtkNew<vtkPoints> points;
  vtkNew<vtkDoubleArray> values;
  values->SetName("values");
  constexpr std::size_t samples = 131073;
  for (std::size_t index = 0; index < samples; ++index)
  {
    points->InsertNextPoint(static_cast<double>(index), 0, 0);
    values->InsertNextValue(static_cast<double>(index % 101));
  }
  mesh->SetPoints(points);
  mesh->Allocate(1);
  const vtkIdType line[] = {0, 1};
  mesh->InsertNextCell(VTK_LINE, 2, line);
  mesh->GetPointData()->AddArray(values);
  SimResults::Field field;
  field.name = "Result";
  field.array_name = "values";
  field.components = {"X"};
  Postprocessor::FieldSelection component;
  component.component = "X";
  PostprocessorReportExtrema parallel;
  check(PostprocessorReportExtrema::collect(mesh, field, component, nullptr, parallel, error) &&
    parallel.available && parallel.minimum == 0 && parallel.maximum == 100 &&
    parallel.minimum_sample == 0 && parallel.maximum_sample == 100,
    "Bounded worker reduction retains original sample indices across chunks");
  SimResults::Dataset block;
  block.id = 0;
  block.kind = "Blocks";
  block.model_id = 17;
  block.name = "Block 17";
  source.datasets = {block};
  for (auto& frame : source.increments)
  {
    frame.fields = {field};
    mesh->Register(nullptr);
    frame.dataset = std::shared_ptr<vtkDataObject>(mesh.GetPointer(), [](vtkDataObject* value)
    {
      value->Delete();
    });
  }
  settings.values["post.results.frames"] = "last_per_step";
  ReportResultRequest request;
  request.settings = settings;
  std::vector<PostprocessorReportBlockSummary> summaries;
  check(PostprocessorReportBlockSummary::collect(request, source, summaries, error) && summaries.size() == 2,
    "Default reporting produces one block/component summary per step");
  settings.values["post.results.frames"] = "envelope";
  request.settings = settings;
  check(PostprocessorReportBlockSummary::collect(request, source, summaries, error) && summaries.size() == 1,
    "Envelope combines the same block/component across selected frames");
  source.datasets.front().kind = "DAT_Elements";
  check(PostprocessorReportBlockSummary::collect(request, source, summaries, error) && summaries.empty(),
    "An arbitrary DAT element set is not mislabeled as a model block");
  return failures;
}
