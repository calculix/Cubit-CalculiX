#include "PostprocessorReportBlockSummary.hpp"
#include "PostprocessorReportFrameSelection.hpp"
#include "PostprocessorReportExtrema.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkVariant.h>
#include <QCryptographicHash>
#include <QString>
#include <cmath>
#include <map>
#include <set>
#include <utility>

namespace
{
bool entity_position(vtkDataSet* mesh, const SimResults::Field& field, std::size_t sample, const SimUnits::QuantityUnit& quantity, std::array<QJsonValue, 3>& output, std::string& unit, std::string& error)
{
  output.fill(QStringLiteral("Unavailable"));
  double unused = 0.0;
  if (!SimResults::ResultsUnits::display_value(0.0, quantity, "report", unused, unit, error))
  {
    return false;
  }
  if (!mesh)
  {
    return true;
  }
  double position[3] = {0.0, 0.0, 0.0};
  if (field.association == SimResults::Association::Element)
  {
    if (sample >= static_cast<std::size_t>(mesh->GetNumberOfCells()))
    {
      return true;
    }
    auto* cell = mesh->GetCell(static_cast<vtkIdType>(sample));
    if (!cell || cell->GetNumberOfPoints() == 0)
    {
      return true;
    }
    double coordinates[3] = {0.0, 0.0, 0.0};
    int subId = cell->GetParametricCenter(coordinates);
    std::vector<double> weights(static_cast<std::size_t>(cell->GetNumberOfPoints()));
    cell->EvaluateLocation(subId, coordinates, position, weights.data());
  }
  else
  {
    if (sample >= static_cast<std::size_t>(mesh->GetNumberOfPoints()))
    {
      return true;
    }
    mesh->GetPoint(static_cast<vtkIdType>(sample), position);
  }
  for (std::size_t axis = 0; axis < output.size(); ++axis)
  {
    double converted = 0.0;
    if (!SimResults::ResultsUnits::display_value(position[axis], quantity, "report", converted, unit, error))
    {
      return false;
    }
    if (std::isfinite(converted))
    {
      output[axis] = converted;
    }
  }
  return true;
}

std::string entity_id(vtkDataSet& mesh, const SimResults::Field& field, std::size_t sample)
{
  vtkDataArray* ids = nullptr;
  if (field.association == SimResults::Association::Element)
  {
    ids = mesh.GetCellData()->GetArray("OriginalElementId");
  }
  else if (field.association == SimResults::Association::Node)
  {
    ids = mesh.GetPointData()->GetArray("OriginalNodeId");
  }
  else
  {
    ids = mesh.GetPointData()->GetArray("OriginalElementId");
  }
  if (!ids || sample >= static_cast<std::size_t>(ids->GetNumberOfTuples()))
  {
    return "unavailable";
  }
  std::string result = ids->GetVariantValue(static_cast<vtkIdType>(sample)).ToString();
  if (field.association == SimResults::Association::IntegrationPoint)
  {
    auto* ip = mesh.GetPointData()->GetArray("IntegrationPointId");
    if (ip && sample < static_cast<std::size_t>(ip->GetNumberOfTuples()))
    {
      result += " / IP " + std::string(ip->GetVariantValue(static_cast<vtkIdType>(sample)).ToString());
    }
  }
  return result;
}

}

PostprocessorReportBlockSummary::PostprocessorReportBlockSummary() = default;

bool PostprocessorReportBlockSummary::source(const PostprocessorApplicationInterface& application,
  const ReportSource& requested, std::shared_ptr<const SimResults::VtkResultsData>& output,
  std::string& error)
{
  std::shared_ptr<const SimResults::VtkResultsData> candidate;
  for (const auto& entry : application.session().sources)
  {
    const auto data = application.result_data(entry.first);
    if (!data || data->job_id != requested.job_id || data->source_workspace != requested.workspace_id ||
        requested.solver_launch_id.empty() || data->solver_launch_id != requested.solver_launch_id)
    {
      continue;
    }
    if (candidate && candidate->result_identity != data->result_identity)
    {
      error = "Several retained result sources match this solver launch.";
      return false;
    }
    candidate = data;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}

bool PostprocessorReportBlockSummary::collect(const ReportResultRequest& request,
  const SimResults::VtkResultsData& source, std::vector<PostprocessorReportBlockSummary>& output,
  std::string& error, std::vector<ReportDiagnostic>* diagnostics)
{
  std::vector<std::size_t> frames;
  if (!PostprocessorReportFrameSelection().select(source, request.settings, frames, error))
  {
    return false;
  }
  std::map<std::size_t, SimResults::Dataset> blocks;
  for (const auto& dataset : source.datasets)
  {
    if (dataset.kind == "Blocks" && dataset.model_id >= 0)
    {
      blocks.emplace(dataset.id, dataset);
    }
  }
  std::vector<ReportDiagnostic> issues;
  std::set<std::string> unmapped;
  for (const auto index : frames)
  {
    for (const auto& field : source.increments[index].fields)
    {
      if (field.categorical || field.association == SimResults::Association::Table ||
          blocks.count(field.dataset_id) != 0)
      {
        continue;
      }
      const auto key = Postprocessor::field_group_key(field) + "/" + std::to_string(field.dataset_id);
      if (unmapped.insert(key).second)
      {
        ReportDiagnostic issue;
        issue.section_id = "post.results";
        issue.severity = "unavailable";
        issue.message = "No verified model-block mapping for " + field.name + " / dataset " +
          std::to_string(field.dataset_id) + "; block extrema and component images were omitted for this field.";
        issues.push_back(std::move(issue));
      }
    }
  }
  const bool envelope = PostprocessorReportFrameSelection::envelope(request.settings);
  std::map<std::string, PostprocessorReportBlockSummary> summaries;
  std::size_t completed = 0;
  std::size_t total = 0;
  for (const auto frameIndex : frames)
  {
    for (const auto& field : source.increments[frameIndex].fields)
    {
      if (!field.categorical && field.association != SimResults::Association::Table &&
          blocks.count(field.dataset_id) != 0)
      {
        total += field.components.size();
      }
    }
  }
  for (const auto frameIndex : frames)
  {
    const auto& frame = source.increments[frameIndex];
    SimUnits::QuantityUnit coordinates("length", frame.coordinate_unit, SimUnits::Length);
    if (coordinates.unit.empty())
    {
      coordinates.unit = source.source_units.coordinate_unit;
    }
    for (const auto& field : frame.fields)
    {
      const auto block = blocks.find(field.dataset_id);
      if (field.categorical || field.association == SimResults::Association::Table || block == blocks.end())
      {
        continue;
      }
      for (const auto& component : field.components)
      {
        std::string key = Postprocessor::field_group_key(field) + "/" +
          std::to_string(field.dataset_id) + "/" + component;
        if (!envelope)
        {
          key += "/frame/" + std::to_string(frameIndex);
        }
        auto& summary = summaries[key];
        summary.id = "post.block." + QCryptographicHash::hash(QByteArray::fromStdString(key),
          QCryptographicHash::Sha256).toHex().toStdString();
        summary.block = block->second;
        if (summary.available)
        {
          SimUnits::QuantityUnit previous, current;
          if (!SimResults::ResultsUnits::field_quantity(summary.field, component, false, previous, error) ||
              !SimResults::ResultsUnits::field_quantity(field, component, false, current, error))
          {
            return false;
          }
          if (previous.dimension != current.dimension || previous.role != current.role ||
              previous.quantity != current.quantity || previous.unit.empty() != current.unit.empty())
          {
            error = "Cannot combine incompatible block component quantities across frames.";
            return false;
          }
        }
        summary.field = field;
        summary.component = component;
        Postprocessor::FieldSelection selection;
        selection.dataset_id = field.dataset_id;
        selection.array_name = field.array_name;
        selection.association = field.association;
        selection.component = component;
        selection.group_key = Postprocessor::field_group_key(field);
        auto raw = field;
        raw.component_units.clear();
        for (const auto& part : Postprocessor::result_mesh_parts(frame.dataset.get()))
        {
          if (part.first != field.dataset_id)
          {
            continue;
          }
          PostprocessorReportExtrema range;
          if (!PostprocessorReportExtrema::collect(part.second, raw, selection, request.progress, range, error))
          {
            return false;
          }
          if (!range.available)
          {
            continue;
          }
          std::string unit;
          double minimum = 0.0, maximum = 0.0;
          if (!SimResults::ResultsUnits::field_value(range.minimum, field, component, false,
                "report", minimum, unit, error) ||
              !SimResults::ResultsUnits::field_value(range.maximum, field, component, false,
                "report", maximum, unit, error))
          {
            return false;
          }
          if (summary.available && summary.unit != unit)
          {
            error = "Cannot combine block extrema with incompatible result units.";
            return false;
          }
          double unused = 0.0;
          std::string coordinateUnit;
          if (!SimResults::ResultsUnits::display_value(0.0, coordinates, "report", unused, coordinateUnit, error))
          {
            return false;
          }
          if (summary.available && summary.coordinate_unit != coordinateUnit)
          {
            error = "Cannot combine extrema locations with incompatible coordinate units.";
            return false;
          }
          summary.unit = unit;
          if (!summary.available || minimum < summary.minimum)
          {
            summary.minimum = minimum;
            summary.minimum_frame = frameIndex;
            summary.minimum_id = entity_id(*part.second, field, range.minimum_sample);
            if (!entity_position(part.second, field, range.minimum_sample, coordinates,
                  summary.minimum_position, summary.coordinate_unit, error))
            {
              return false;
            }
          }
          if (!summary.available || maximum > summary.maximum)
          {
            summary.maximum = maximum;
            summary.maximum_frame = frameIndex;
            summary.maximum_id = entity_id(*part.second, field, range.maximum_sample);
            if (!entity_position(part.second, field, range.maximum_sample, coordinates,
                  summary.maximum_position, summary.coordinate_unit, error))
            {
              return false;
            }
          }
          summary.available = true;
        }
        if (!ReportProgress::advance(request.progress, ReportProgressStage::Results,
            "Collecting block " + std::to_string(summary.block.model_id) + " / " + component,
            ++completed, total, error))
        {
          return false;
        }
      }
    }
  }
  std::vector<PostprocessorReportBlockSummary> candidate;
  for (auto& entry : summaries)
  {
    candidate.push_back(std::move(entry.second));
  }
  if (diagnostics)
  {
    diagnostics->insert(diagnostics->end(), issues.begin(), issues.end());
  }
  output = std::move(candidate);
  error.clear();
  return true;
}

ReportSection PostprocessorReportBlockSummary::section(bool values) const
{
  ReportSection result;
  result.id = id;
  result.title = "Block " + std::to_string(block.model_id) + " / " + block.name +
    " / " + field.name + " / " + component;
  result.paragraphs.push_back("Locations use undeformed source coordinates; element positions are parametric centers. Result positions retain original entity IDs.");
  if (!available)
  {
    result.paragraphs.push_back("No finite valid samples are available for this selection.");
    return result;
  }
  if (values)
  {
    ReportTable table;
    table.title = "Block extrema";
    table.columns = {"Extremum", "Value", "Entity / integration point", "X", "Y", "Z", "Source frame"};
    table.units = {"", unit, "", coordinate_unit, coordinate_unit, coordinate_unit, ""};
    SimUnits::QuantityUnit valueQuantity;
    std::string ignored;
    if (SimResults::ResultsUnits::field_quantity(field, component, false, valueQuantity, ignored) &&
        !valueQuantity.unit.empty() && unit != "unknown unit")
    {
      valueQuantity.unit = unit;
      table.column_quantities.resize(table.columns.size());
      table.column_quantities[1] = ReportQuantity(valueQuantity);
      if (coordinate_unit != "unknown unit" && !coordinate_unit.empty())
      {
        const SimUnits::QuantityUnit coordinates("length", coordinate_unit, SimUnits::Length);
        for (std::size_t axis = 3; axis < 6; ++axis)
        {
          table.column_quantities[axis] = ReportQuantity(coordinates);
        }
      }
    }
    table.rows = {
      {"Minimum", minimum, QString::fromStdString(minimum_id), minimum_position[0], minimum_position[1], minimum_position[2], static_cast<double>(minimum_frame + 1)},
      {"Maximum", maximum, QString::fromStdString(maximum_id), maximum_position[0], maximum_position[1], maximum_position[2], static_cast<double>(maximum_frame + 1)}};
    result.tables.push_back(std::move(table));
  }
  return result;
}
