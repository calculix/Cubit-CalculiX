#include "PlotData.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <utility>
#include "../representations/ResultMesh.hpp"
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  bool plot_entity_data(const SimResults::VtkResultsData& data, const PlotEntityReference& reference,
    PlotCurveData& output, std::string& error) {
    error.clear();
    output.samples.clear();
    output.status.clear();
    output.x_title.clear();
    output.y_title.clear();
    output.time_axis = output.frame_axis = false;
    bool invalidIntegrationPoint = reference.entity.integration_point.has_value();
    if (reference.entity.kind == EntityKind::IntegrationPoint) {
      invalidIntegrationPoint = !reference.entity.integration_point || *reference.entity.integration_point < 1 || *reference.entity.integration_point > 2147483647;
    }
    if ((reference.entity.kind != EntityKind::Node && reference.entity.kind != EntityKind::Element &&
        reference.entity.kind != EntityKind::IntegrationPoint) || reference.entity.entity_id < 0 ||
        invalidIntegrationPoint ||
        (reference.reduction != PlotReduction::None && reference.reduction != PlotReduction::Minimum && reference.reduction != PlotReduction::Maximum)) {
      error = "The history entity identity or reduction is invalid.";
      return false;
    }
    if (data.increments.size() > PlotDefaults::maximum_samples) {
      error = "The history exceeds the per-curve sample limit; no samples were truncated.";
      return false;
    }
    std::optional<std::size_t> dataset;
    for (const auto& item : result_dataset_catalogue(data)) {
      bool identityMatches = item.name == reference.dataset_name;
      if (reference.model_id >= 0) {
        identityMatches = item.model_id == reference.model_id;
      }
      if (item.kind == reference.dataset_kind && identityMatches) {
        if (dataset) {
          output.status = "Dataset identity is ambiguous.";
          return true;
        }
        dataset = item.id;
      }
    }
    if (!dataset) {
      output.status = "Dataset is unavailable.";
      return true;
    }
    auto entity = reference.entity;
    entity.dataset_id = *dataset;
    Representation display;
    display.coloring.field = reference.field;
    display.coloring.field->dataset_id = *dataset;
    output.x_title = "Time";
    std::string component = reference.field.component;
    if (reference.field.magnitude) {
      component = "Magnitude";
    }
    output.y_title = reference.field.array_name + " / " + component;
    output.time_axis = true;
    output.samples.reserve(data.increments.size());
    for (std::size_t index = 0; index < data.increments.size(); ++index) {
      display.increment_index = index;
      const auto& frame = data.increments[index];
      if (frame.independent_variable.quantity != "time") {
        output.x_title = "Analysis parameter";
      }
      PlotSample sample;
      sample.x = frame.time;
      sample.time = frame.time;
      if (!SimResults::ResultsUnits::display_value(frame.time, frame.independent_variable, "postprocessor", sample.x, output.x_unit, error)) {
        return false;
      }
      sample.time = sample.x;
      output.time_unit = output.x_unit;
      sample.frame = index;
      sample.step = frame.step;
      sample.increment = frame.increment;
      const auto result = probe_result(data,display,entity);
      output.y_unit = result.value_unit;
      if (result.values) {
        if (result.range_value && reference.reduction == PlotReduction::None) {
          error = "An element with nodal results requires reduction minimum or maximum.";
          return false;
        }
        std::size_t reduction = 0;
        if (reference.reduction == PlotReduction::Maximum) {
          reduction = 1;
        }
        sample.y = (*result.values)[reduction];
        sample.valid = std::isfinite(sample.x) && std::isfinite(sample.y);
      }
      output.samples.push_back(sample);
    }
    if (std::none_of(output.samples.begin(),output.samples.end(),[](const PlotSample& sample) {
        return sample.valid;
      })) {
      output.status = "No valid original samples are available.";
    }
    output.x_title += " [" + output.x_unit + "]";
    output.y_title += " [" + output.y_unit + "]";
    return true;
  }
}
