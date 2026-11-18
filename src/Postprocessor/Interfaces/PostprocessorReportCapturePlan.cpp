#include "PostprocessorReportCapturePlan.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Core/representations/ResultColoringUnits.hpp"
#include "Postprocessor/Core/representations/VectorPlotCodec.hpp"
#include "Postprocessor/Core/persistence/DatasetAppearanceCodec.hpp"
#include "Postprocessor/Core/filters/FilterCodec.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <QJsonArray>
#include <algorithm>
#include <set>
#include <utility>

namespace
{
  std::string setting(const ReportConfiguration& settings, const std::string& key,
    const std::string& fallback)
  {
    const auto found = settings.values.find(key);
    if (found != settings.values.end())
    {
      return found->second;
    }
    return fallback;
  }

  // Walk only upstream filter definitions. Reject cycles/dangling references;
  // never substitute a direct source for an unavailable filtered output.
  bool source_of(const Postprocessor::CaptureSnapshot& snapshot,
    Postprocessor::OutputReference input, std::string& source,
    std::set<std::string>& filters, std::string& error)
  {
    while (input.owner == Postprocessor::OutputOwner::Feature)
    {
      const auto found = snapshot.session.filters.find(input.object_id);
      if (found == snapshot.session.filters.end() || !filters.insert(input.object_id).second)
      {
        error = "The report presentation has a missing or cyclic filter dependency.";
        return false;
      }
      input = found->second.input;
    }
    source = input.object_id;
    return true;
  }

  QJsonArray retained_manifest_entries(const QJsonArray& entries, const std::string& id)
  {
    QJsonArray result;
    for (const auto& entry : entries)
    {
      if (entry.toObject().value("id").toString() == QString::fromStdString(id))
      {
        result.append(entry);
      }
    }
    return result;
  }
}

bool PostprocessorReportCapturePlan::initialize(Postprocessor::CaptureSnapshot snapshot,
  const std::string& source, const ReportConfiguration& settings, std::string& error)
{
  error.clear();
  const auto loaded = snapshot.sources.find(source);
  const auto view = snapshot.session.views.find(snapshot.target_id);
  if (loaded == snapshot.sources.end() || !loaded->second || view == snapshot.session.views.end() ||
      snapshot.session.sources.count(source) == 0)
  {
    error = "The report capture source or view is unavailable.";
    return false;
  }
  const auto mode = setting(settings, "post.images.presentation", "inherited");
  const auto camera = setting(settings, "post.images.camera", "preserve");
  const auto background = setting(settings, "post.images.background", "current");
  if ((mode != "inherited" && mode != "default") || (camera != "preserve" && camera != "fit") ||
      (background != "current" && background != "white" && background != "transparent"))
  {
    error = "Invalid report image presentation, camera or background option.";
    return false;
  }

  std::vector<std::string> matches;
  if (mode == "inherited")
  {
    for (const auto& item : snapshot.session.representations)
    {
      if (item.second.view_id != snapshot.target_id || !item.second.visible)
      {
        continue;
      }
      std::string resolved;
      std::set<std::string> dependencies;
      if (!source_of(snapshot, item.second.input, resolved, dependencies, error))
      {
        return false;
      }
      if (resolved == source)
      {
        matches.push_back(item.first);
      }
    }
  }
  std::string selected;
  for (const auto& id : matches)
  {
    if (id == snapshot.session.active_representation_id)
    {
      selected = id;
      break;
    }
  }
  if (selected.empty() && matches.size() == 1)
  {
    selected = matches.front();
  }
  if (selected.empty() && matches.size() > 1)
  {
    error = "Multiple report presentations match this source; activate the intended representation in Postprocessor.";
    return false;
  }

  Postprocessor::Representation display;
  const bool useInherited = !selected.empty();
  if (useInherited)
  {
    display = snapshot.session.representations.at(selected);
  }
  else
  {
    display.id = "report.result";
    display.name = "Report result";
    display.input.object_id = source;
    display.view_id = snapshot.target_id;
    display.coloring.range_mode = Postprocessor::ColorRangeMode::Automatic;
  }
  // Bind legacy/unbound limits while the original presentation is still active,
  // before any progress callbacks can observe later live unit changes.
  if (display.coloring.field && !display.coloring.bounds_unit &&
      display.increment_index < loaded->second->increments.size())
  {
    const auto* original = Postprocessor::find_field(
      loaded->second->increments[display.increment_index], *display.coloring.field);
    if (original && !Postprocessor::bind_coloring_units(display.coloring, *original, error))
    {
      return false;
    }
  }
  std::set<std::string> dependencies;
  std::string resolved;
  if (!source_of(snapshot, display.input, resolved, dependencies, error) || resolved != source)
  {
    if (error.empty())
    {
      error = "The selected report presentation belongs to another result source.";
    }
    return false;
  }
  for (auto item = snapshot.session.filters.begin(); item != snapshot.session.filters.end();)
  {
    if (dependencies.count(item->first) == 0)
    {
      item = snapshot.session.filters.erase(item);
    }
    else
    {
      ++item;
    }
  }
  auto retainedView = view->second;
  retainedView.fit_pending = false;
  retainedView.plot_active = false;
  retainedView.plot_id.reset();
  retainedView.table.reset();
  if (camera == "fit")
  {
    retainedView.camera.reset();
  }
  // Camera positions and lengths remain in this exact scene basis. Presentation
  // unit preferences below must never rescale or replace that coordinate basis.
  if (retainedView.scene_unit.empty() && !retainedView.camera)
  {
    retainedView.scene_unit = loaded->second->source_units.coordinate_unit;
  }
  const auto retainedSource = snapshot.session.sources.at(source);
  const auto retainedData = loaded->second;
  snapshot.sources.clear();
  snapshot.sources.emplace(source, retainedData);
  snapshot.session.sources.clear();
  snapshot.session.sources.emplace(source, retainedSource);
  snapshot.session.representations.clear();
  snapshot.session.representations.emplace(display.id, display);
  snapshot.session.views.clear();
  snapshot.session.views.emplace(snapshot.target_id, std::move(retainedView));
  snapshot.session.plots.clear();
  snapshot.session.active_plot_id.clear();
  snapshot.session.active_source_id = source;
  snapshot.session.active_representation_id = display.id;
  snapshot.request.background = Postprocessor::CaptureBackground::Current;
  if (background == "white")
  {
    snapshot.request.background = Postprocessor::CaptureBackground::White;
    snapshot.options.backgroundTop = snapshot.options.backgroundBottom = {1, 1, 1};
  }
  else if (background == "transparent")
  {
    snapshot.request.background = Postprocessor::CaptureBackground::Transparent;
  }
  if (!framework_units().workspace_context("report", snapshot.presentation_units, error))
  {
    return false;
  }
  snapshot.presentation_units.workspace = "postprocessor";
  auto originalProvenance = snapshot.provenance;
  originalProvenance.insert("sources", retained_manifest_entries(
    originalProvenance.value("sources").toArray(), source));
  originalProvenance.insert("representations", retained_manifest_entries(
    originalProvenance.value("representations").toArray(), display.id));
  originalProvenance.insert("views", retained_manifest_entries(
    originalProvenance.value("views").toArray(), snapshot.target_id));
  originalProvenance.insert("filters", Postprocessor::filters_to_json(snapshot.session.filters));
  snapshot.provenance.remove("representations");
  snapshot.provenance.remove("sources");
  snapshot.provenance.remove("views");
  snapshot.provenance.insert("inherited_presentation", originalProvenance);
  snapshot.provenance.insert("report_presentation", QString::fromStdString(mode));
  snapshot.provenance.insert("background", static_cast<int>(snapshot.request.background));
  snapshot.provenance.insert("presentation_fallback", mode == "inherited" && !useInherited);
  snapshot.provenance.insert("filters", Postprocessor::filters_to_json(snapshot.session.filters));
  base = std::move(snapshot);
  sourceId = source;
  representationId = display.id;
  inherited = useInherited;
  error.clear();
  return true;
}

bool PostprocessorReportCapturePlan::prepare(std::size_t frame, std::size_t dataset,
  const Postprocessor::FieldSelection& field, Postprocessor::CaptureSnapshot& output,
  std::string& error) const
{
  error.clear();
  const auto loaded = base.sources.find(sourceId);
  if (loaded == base.sources.end() || !loaded->second || frame >= loaded->second->increments.size() ||
      base.session.representations.count(representationId) == 0)
  {
    error = "The report result frame or presentation is unavailable.";
    return false;
  }
  const auto& data = *loaded->second;
  const auto& increment = data.increments[frame];
  const auto* resultField = Postprocessor::find_field(increment, field);
  const auto parts = Postprocessor::result_mesh_parts(increment.dataset.get());
  const bool hasBlock = std::any_of(parts.begin(), parts.end(), [dataset](const auto& part)
  {
    return part.first == dataset;
  });
  if (!resultField || !hasBlock || (field.dataset_id && *field.dataset_id != dataset))
  {
    error = "The report block or component is unavailable in this frame.";
    return false;
  }
  auto candidate = base;
  auto& display = candidate.session.representations.at(representationId);
  auto coloring = display.coloring;
  // Bounds retain their original quantity and temperature role. A different
  // quantity cannot inherit numerical limits, even when magnitudes look similar.
  SimUnits::QuantityUnit quantity;
  if (!SimResults::ResultsUnits::field_quantity(*resultField, field.component, field.magnitude, quantity, error))
  {
    return false;
  }
  const bool compatible = coloring.bounds_unit && coloring.bounds_unit->quantity == quantity.quantity &&
    coloring.bounds_unit->dimension == quantity.dimension && coloring.bounds_unit->role == quantity.role &&
    coloring.bounds_unit->unit.empty() == quantity.unit.empty();
  bool adjusted = false;
  if (!compatible)
  {
    adjusted = inherited && (coloring.range_mode == Postprocessor::ColorRangeMode::Fixed ||
      coloring.minimum.enabled || coloring.maximum.enabled);
    coloring.range_mode = Postprocessor::ColorRangeMode::Automatic;
    coloring.minimum.enabled = false;
    coloring.maximum.enabled = false;
    coloring.bounds_unit = quantity;
  }
  coloring.field = field;
  auto units = framework_units();
  if (!units.set_workspace_context(candidate.presentation_units, error) ||
      !Postprocessor::convert_coloring_units(coloring, units, error))
  {
    return false;
  }
  display.coloring = std::move(coloring);
  display.increment_index = frame;
  display.animation.playing = false;
  display.visible = true;
  display.hidden_datasets.clear();
  for (const auto& item : data.datasets)
  {
    if (item.id != dataset)
    {
      display.hidden_datasets.insert(item.id);
    }
  }
  for (const auto& part : parts)
  {
    if (part.first != dataset)
    {
      display.hidden_datasets.insert(part.first);
    }
  }
  QJsonArray appearances;
  for (const auto& item : display.dataset_appearance)
  {
    appearances.append(QJsonObject{{"dataset", QString::number(static_cast<qulonglong>(item.first))},
      {"appearance", Postprocessor::appearance_to_json(item.second)}});
  }
  const bool filtered = std::any_of(candidate.session.filters.begin(), candidate.session.filters.end(),
    [](const auto& item)
  {
    return item.second.enabled;
  });
  // Keep the original capture manifest as provenance, while making the adapted
  // image's authoritative representation/frame/scope explicit in a separate record.
  candidate.provenance.insert("report_capture", QJsonObject{
    {"source", QString::fromStdString(sourceId)},
    {"representation", QString::fromStdString(representationId)},
    {"inherited", inherited}, {"frame", static_cast<double>(frame + 1)},
    {"dataset", static_cast<double>(dataset)},
    {"result_identity", QString::fromStdString(data.result_identity)},
    {"solver_launch_id", QString::fromStdString(data.solver_launch_id)},
    {"field", QString::fromStdString(field.group_key)},
    {"component", QString::fromStdString(field.component)},
    {"appearances", appearances}, {"vectors", Postprocessor::vector_plot_to_json(display.vectors)},
    {"filtered", filtered}, {"automatic_range_adjustment", adjusted},
    {"extrema_scope", "complete original block"}});
  QJsonObject preferences;
  for (const auto& item : candidate.presentation_units.display_units)
  {
    preferences.insert(QString::fromStdString(item.first), QString::fromStdString(item.second));
  }
  candidate.provenance.insert("presentation_units", QJsonObject{
    {"workspace", "report"}, {"system", QString::fromStdString(candidate.presentation_units.system)},
    {"display_units", preferences}});
  output = std::move(candidate);
  error.clear();
  return true;
}
