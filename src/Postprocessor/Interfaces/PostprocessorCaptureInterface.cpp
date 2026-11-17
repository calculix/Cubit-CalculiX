#include <type_traits>
#include <utility>
#include "PostprocessorApplicationInterface.hpp"
#include "PostprocessorReportBlockSummary.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
void PostprocessorApplicationInterface::set_capture_renderer(CaptureRenderer renderer,CaptureSizer sizer)
{
  captureRenderer = std::move(renderer);
  captureSizer = std::move(sizer);
}
std::array<int,2> PostprocessorApplicationInterface::capture_reference_size(
  Postprocessor::CaptureTarget target,const std::string& id) const
{
  const auto size = [&]() -> std::common_type_t<decltype(captureSizer(target,id)), decltype(std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height})> {
    if (captureSizer) {
      return captureSizer(target,id);
    } else {
      return std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
    }
  }();
  if (size[0] >= 64 && size[1] >= 64) {
    return size;
  } else {
    return std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
  }
}
bool PostprocessorApplicationInterface::render_capture(const Postprocessor::CaptureDefinition& request,
  QByteArray& png,QJsonObject& provenance,std::string& error) const
{
  error.clear();
  if (!captureRenderer)
  {
    error = "PNG capture requires the active Cubit GUI rendering service.";
    return false;
  }
  Postprocessor::CaptureSnapshot snapshot;
  if (!core.capture_snapshot(request,snapshot,error))
  {
    return false;
  }
  snapshot.options = user_options();
  if (request.triad)
  {
    snapshot.options.triadVisible = *request.triad;
  }
  if (request.background == Postprocessor::CaptureBackground::White)
  {
    snapshot.options.backgroundTop = snapshot.options.backgroundBottom = {1,1,1};
  }
  snapshot.reference_size = capture_reference_size(request.target,snapshot.target_id);
  auto frozen = std::make_shared<PostprocessorApplicationInterface>(providers);
  frozen->core.install_capture_snapshot(snapshot);
  frozen->captureOptions = snapshot.options;
  QByteArray candidate;
  QJsonObject manifest;
  if (!captureRenderer(snapshot,frozen,candidate,manifest,error))
  {
    return false;
  }
  png = std::move(candidate);
  provenance = std::move(manifest);
  return true;
}
bool PostprocessorApplicationInterface::capture_file(const Postprocessor::CaptureDefinition& request,
  const std::string& path,std::string& error) const
{
  QByteArray png;
  QJsonObject provenance;
  return render_capture(request,png,provenance,error) && Postprocessor::write_capture_png(path,png,error);
}

const SimUnits::WorkspaceUnitContext* PostprocessorApplicationInterface::captured_units() const noexcept
{
  return core.captured_units();
}

bool PostprocessorApplicationInterface::render_report_result(const ReportSource& requested,
  std::size_t frame, std::size_t dataset, const Postprocessor::FieldSelection& selection,
  ReportImage& output, std::string& error) const
{
  if (!captureRenderer)
  {
    error = "Result images require the Cubit GUI capture renderer.";
    return false;
  }
  std::shared_ptr<const SimResults::VtkResultsData> data;
  if (!PostprocessorReportBlockSummary::source(*this, requested, data, error))
  {
    return false;
  }
  if (!data || frame >= data->increments.size())
  {
    error = "The report result frame is unavailable.";
    return false;
  }
  std::string sourceId;
  for (const auto& entry : session().sources)
  {
    if (result_data(entry.first) == data)
    {
      sourceId = entry.first;
      break;
    }
  }
  Postprocessor::CaptureDefinition definition;
  definition.legend = true;
  definition.labels = true;
  definition.background = Postprocessor::CaptureBackground::White;
  Postprocessor::CaptureSnapshot snapshot;
  if (!core.capture_snapshot(definition, snapshot, error) ||
      !framework_units().workspace_context("report", snapshot.presentation_units, error))
  {
    return false;
  }
  // Rendering calls the Postprocessor unit API. Rebind only the frozen context's
  // workspace name; its system/display preferences remain the Report settings.
  snapshot.presentation_units.workspace = "postprocessor";
  const auto description = snapshot.session.sources.find(sourceId);
  if (description == snapshot.session.sources.end())
  {
    error = "The report result source description is unavailable.";
    return false;
  }
  const auto retained = description->second;
  snapshot.session.sources.clear();
  snapshot.session.sources.emplace(sourceId, retained);
  snapshot.sources.clear();
  snapshot.sources.emplace(sourceId, data);
  snapshot.session.filters.clear();
  snapshot.session.plots.clear();
  snapshot.session.active_plot_id.clear();
  snapshot.session.active_source_id = sourceId;
  snapshot.session.representations.clear();
  Postprocessor::Representation display;
  display.id = "report.result";
  display.name = "Report result";
  display.input.object_id = sourceId;
  display.view_id = snapshot.target_id;
  display.increment_index = frame;
  display.coloring.field = selection;
  display.coloring.range_mode = Postprocessor::ColorRangeMode::Automatic;
  for (const auto& item : data->datasets)
  {
    if (item.id != dataset)
    {
      display.hidden_datasets.insert(item.id);
    }
  }
  for (const auto& part : Postprocessor::result_mesh_parts(data->increments[frame].dataset.get()))
  {
    if (part.first != dataset)
    {
      display.hidden_datasets.insert(part.first);
    }
  }
  snapshot.session.representations.emplace(display.id, display);
  snapshot.session.active_representation_id = display.id;
  auto view = snapshot.session.views.at(snapshot.target_id);
  view.camera.reset();
  view.fit_pending = false;
  view.plot_id.reset();
  view.plot_active = false;
  view.table.reset();
  view.annotations.labels.clear();
  view.annotations.visible = true;
  view.annotations.extrema_visible = true;
  view.scene_unit = data->increments[frame].coordinate_unit;
  if (view.scene_unit.empty())
  {
    view.scene_unit = data->source_units.coordinate_unit;
  }
  snapshot.session.views.clear();
  snapshot.session.views.emplace(snapshot.target_id, view);
  snapshot.options = user_options();
  snapshot.options.backgroundTop = snapshot.options.backgroundBottom = {1, 1, 1};
  snapshot.provenance = QJsonObject{
    {"target", "view"}, {"id", QString::fromStdString(snapshot.target_id)},
    {"width", definition.width}, {"height", definition.height},
    {"result_identity", QString::fromStdString(data->result_identity)},
    {"solver_launch_id", QString::fromStdString(data->solver_launch_id)},
    {"frame", static_cast<double>(frame + 1)}, {"dataset", static_cast<double>(dataset)},
    {"field", QString::fromStdString(selection.group_key)},
    {"component", QString::fromStdString(selection.component)},
    {"unit_workspace", "report"}};
  QJsonObject preferences;
  for (const auto& item : snapshot.presentation_units.display_units)
  {
    preferences.insert(QString::fromStdString(item.first), QString::fromStdString(item.second));
  }
  snapshot.provenance.insert("presentation_units", QJsonObject{
    {"system", QString::fromStdString(snapshot.presentation_units.system)},
    {"display_units", preferences}});
  snapshot.reference_size = {definition.width, definition.height};
  auto frozen = std::make_shared<PostprocessorApplicationInterface>(providers);
  frozen->core.install_capture_snapshot(snapshot);
  frozen->captureOptions = snapshot.options;
  QByteArray png;
  ReportImage candidate;
  if (!captureRenderer(snapshot, frozen, png, candidate.provenance, error))
  {
    return false;
  }
  candidate.media_type = "image/png";
  candidate.base64_data = png.toBase64().toStdString();
  output = std::move(candidate);
  error.clear();
  return true;
}
