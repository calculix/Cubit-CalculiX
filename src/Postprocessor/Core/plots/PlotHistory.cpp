#include "PlotData.hpp"
#include <utility>
#include "../representations/ResultMesh.hpp"
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  bool plot_entity_data(const SimResults::VtkResultsData& data, const PlotEntityReference& reference,
    PlotCurveData& output, std::string& error)
  {
    error.clear();
    output.samples.clear();
    output.status.clear();
    output.x_title.clear();
    output.y_title.clear();
    output.time_axis = output.frame_axis = false;
    if ((reference.entity.kind != EntityKind::Node && reference.entity.kind != EntityKind::Element &&
        reference.entity.kind != EntityKind::IntegrationPoint) || reference.entity.entity_id < 0 ||
        (reference.entity.kind == EntityKind::IntegrationPoint ? !reference.entity.integration_point ||
          *reference.entity.integration_point < 1 || *reference.entity.integration_point > 2147483647 : reference.entity.integration_point.has_value()) ||
        (reference.reduction != PlotReduction::None && reference.reduction != PlotReduction::Minimum && reference.reduction != PlotReduction::Maximum))
    {
      error = "The history entity identity or reduction is invalid.";
      return false;
    }
    if (data.increments.size() > PlotDefaults::maximum_samples)
    {
      error = "The history exceeds the per-curve sample limit; no samples were truncated.";
      return false;
    }
    std::optional<std::size_t> dataset;
    for (const auto& item : result_dataset_catalogue(data))
    {
      if (item.kind == reference.dataset_kind &&
          (reference.model_id >= 0 ? item.model_id == reference.model_id : item.name == reference.dataset_name))
      {
        if (dataset)
        {
          output.status = "Dataset identity is ambiguous.";
          return true;
        }
        dataset = item.id;
      }
    }
    if (!dataset)
    {
      output.status = "Dataset is unavailable.";
      return true;
    }
    auto entity = reference.entity;
    entity.dataset_id = *dataset;
    Representation display;
    display.coloring.field = reference.field;
    display.coloring.field->dataset_id = *dataset;
    output.x_title = "Time";
    output.y_title = reference.field.array_name+" / "+(reference.field.magnitude ? "Magnitude" : reference.field.component);
    output.time_axis = true;
    output.samples.reserve(data.increments.size());
    for (std::size_t index = 0; index < data.increments.size(); ++index)
    {
      display.increment_index = index;
      const auto& frame = data.increments[index];
      PlotSample sample;
      sample.x = frame.time;
      sample.time = frame.time;
      sample.frame = index;
      sample.step = frame.step;
      sample.increment = frame.increment;
      const auto result = probe_result(data,display,entity);
      if (result.values)
      {
        if (result.range_value && reference.reduction == PlotReduction::None)
        {
          error = "An element with nodal results requires reduction minimum or maximum.";
          return false;
        }
        sample.y = (*result.values)[reference.reduction == PlotReduction::Maximum ? 1 : 0];
        sample.valid = std::isfinite(sample.x) && std::isfinite(sample.y);
      }
      output.samples.push_back(sample);
    }
    if (std::none_of(output.samples.begin(),output.samples.end(),[](const PlotSample& sample)
      {
        return sample.valid;
      }))
    {
      output.status = "No valid original samples are available.";
    }
    return true;
  }
}
