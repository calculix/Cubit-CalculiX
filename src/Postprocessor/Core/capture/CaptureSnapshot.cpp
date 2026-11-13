#include "CaptureDefinition.hpp"
#include "../application/PostprocessorCore.hpp"
#include "../plots/PlotCodec.hpp"
#include "../representations/ResultMesh.hpp"
#include <QJsonArray>
#include <algorithm>
namespace Postprocessor
{
  bool valid_capture(const CaptureDefinition& value,std::string& error)
  {
    error.clear();
    if ((value.target != CaptureTarget::View && value.target != CaptureTarget::Plot) ||
        (value.background != CaptureBackground::Current && value.background != CaptureBackground::White &&
          value.background != CaptureBackground::Transparent) ||
        value.width < CaptureDefaults::minimum_side || value.height < CaptureDefaults::minimum_side ||
        value.width > CaptureDefaults::maximum_side || value.height > CaptureDefaults::maximum_side ||
        static_cast<std::int64_t>(value.width)*value.height > CaptureDefaults::maximum_pixels)
    {
      error = "Capture requires 64..8192 pixels per side and at most 32 million pixels.";
      return false;
    }
    if ((value.target == CaptureTarget::Plot && (value.labels || value.triad)) ||
        (value.target == CaptureTarget::View && value.marker))
    {
      error = "Labels/triad apply to views; marker applies to plots.";
      return false;
    }
    return true;
  }
  bool PostprocessorCore::capture_snapshot(const CaptureDefinition& request,CaptureSnapshot& output,std::string& error) const
  {
    error.clear();
    if (source_operation_active || !valid_capture(request,error))
    {
      if (error.empty())
      {
        error = "Cannot capture during a source operation.";
      }
      return false;
    }
    CaptureSnapshot snapshot;
    snapshot.request = request;
    snapshot.session = state;
    if (request.target == CaptureTarget::Plot)
    {
      snapshot.target_id = plot_id_from_name(request.name);
      if (state.plots.count(snapshot.target_id) == 0)
      {
        error = "The requested plot is unavailable.";
        return false;
      }
      snapshot.provenance.insert("plot",plots_to_json({{snapshot.target_id,state.plots.at(snapshot.target_id)}}));
    }
    else
    {
      if (request.name.empty())
      {
        snapshot.target_id = state.active_view_id;
      }
      else
      {
        for (const auto& view : state.views)
        {
          if (view.first == request.name || view.second.name == request.name)
          {
            if (!snapshot.target_id.empty())
            {
              error = "The capture view name is ambiguous; use its ID.";
              return false;
            }
            snapshot.target_id = view.first;
          }
        }
      }
      if (state.views.count(snapshot.target_id) == 0)
      {
        error = "The requested view is unavailable.";
        return false;
      }
      snapshot.session.active_view_id = snapshot.target_id;
      auto& view = snapshot.session.views.at(snapshot.target_id);
      view.selection.entity.reset();
      view.fit_pending = false;
      if (request.labels)
      {
        view.annotations.visible = *request.labels;
      }
    }
    QJsonArray sources, displays;
    for (const auto& entry : state.sources)
    {
      const auto data = result_data(entry.first);
      if (data)
      {
        snapshot.sources.emplace(entry.first,data);
        QJsonArray files;
        for (const auto& file : data->input_files)
        {
          files.append(QString::fromUtf8(file.c_str()));
        }
        sources.append(QJsonObject{{"id",QString::fromStdString(entry.first)},{"name",QString::fromUtf8(entry.second.name.c_str())},
          {"result_identity",QString::fromStdString(data->result_identity)},{"run_identity",QString::fromStdString(data->run_identity)},
          {"job_id",data->job_id},{"input_files",files}});
      }
    }
    for (auto& entry : snapshot.session.representations)
    {
      auto& display = entry.second;
      display.animation.playing = false;
      if (request.legend)
      {
        display.coloring.legend_visible = *request.legend;
      }
      QJsonArray hidden;
      for (auto id : display.hidden_datasets)
      {
        hidden.append(QString::number(static_cast<qulonglong>(id)));
      }
      QJsonObject item{{"id",QString::fromStdString(entry.first)},{"source",QString::fromStdString(display.input.object_id)},
        {"view",QString::fromStdString(display.view_id)},{"visible",display.visible},
        {"frame",QString::number(static_cast<qulonglong>(display.increment_index+1))},{"hidden",hidden},
        {"palette",QString::fromStdString(color_palette_name(display.coloring.palette))},{"range_mode",static_cast<int>(display.coloring.range_mode)},
        {"fixed_range",QJsonArray{display.coloring.fixed_range[0],display.coloring.fixed_range[1]}},
        {"legend",display.coloring.legend_visible},{"deformation",display.deformation.enabled},{"deformation_scale",display.deformation.scale}};
      if (display.coloring.field)
      {
        const auto& field = *display.coloring.field;
        item.insert("field",QString::fromStdString(field_selection_key(field)));
        item.insert("component",QString::fromStdString(field.component));
        item.insert("magnitude",field.magnitude);
        item.insert("association",static_cast<int>(field.association));
      }
      if (display.deformation.field)
      {
        item.insert("deformation_field",QString::fromStdString(field_selection_key(*display.deformation.field)));
      }
      const auto limit = [](const ColorLimit& value)
      {
        return QJsonObject{{"enabled",value.enabled},{"value",value.value},
          {"color",QJsonArray{value.color[0],value.color[1],value.color[2]}}};
      };
      item.insert("reversed",display.coloring.reversed);
      item.insert("bands",display.coloring.bands ? QJsonValue(static_cast<int>(*display.coloring.bands)) : QJsonValue(QJsonValue::Null));
      item.insert("undeformed",display.deformation.overlay);
      const auto source = snapshot.sources.find(display.input.object_id);
      if (source != snapshot.sources.end() && display.increment_index < source->second->increments.size())
      {
        const auto& frame = source->second->increments[display.increment_index];
        item.insert("step",frame.step);
        item.insert("increment",frame.increment);
        item.insert("time",frame.time);
      }
      item.insert("minimum_limit",limit(display.coloring.minimum));
      item.insert("maximum_limit",limit(display.coloring.maximum));
      displays.append(item);
    }
    snapshot.provenance.insert("version",1);
    snapshot.provenance.insert("target",request.target == CaptureTarget::View ? "view" : "plot");
    snapshot.provenance.insert("id",QString::fromStdString(snapshot.target_id));
    snapshot.provenance.insert("width",request.width);
    snapshot.provenance.insert("height",request.height);
    snapshot.provenance.insert("background",static_cast<int>(request.background));
    snapshot.provenance.insert("sources",sources);
    snapshot.provenance.insert("representations",displays);
    output = std::move(snapshot);
    error.clear();
    return true;
  }
  void PostprocessorCore::install_capture_snapshot(const CaptureSnapshot& snapshot)
  {
    state = snapshot.session;
    runtime.clear();
    for (const auto& source : snapshot.sources)
    {
      SourceRuntime value(source.second,1,first_mesh_increment(*source.second));
      runtime.emplace(source.first,std::move(value));
    }
    ++scene_revision;
  }
}
