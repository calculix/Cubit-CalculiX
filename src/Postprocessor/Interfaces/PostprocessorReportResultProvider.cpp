#include "PostprocessorReportResultProvider.hpp"
#include "PostprocessorReportFrameSelection.hpp"
#include "PostprocessorReportBlockSummary.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportResultRequest.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <vtkAbstractArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkTable.h>
#include <vtkVariant.h>
#include <QStringList>
#include <cmath>
#include <array>
#include <limits>
#include <map>
#include <utility>

namespace {
bool report_cell(vtkAbstractArray& array, vtkIdType row, const SimUnits::QuantityUnit& quantity, QJsonValue& output, std::string& error) {
  const int components = array.GetNumberOfComponents();
  QStringList text;
  for (int component = 0; component < components; ++component) {
    const auto cell = array.GetVariantValue(row * components + component);
    QJsonValue value(QStringLiteral("Unavailable"));
    const bool floating = array.GetDataType() == VTK_FLOAT || array.GetDataType() == VTK_DOUBLE;
    const bool physical = array.IsNumeric() && !quantity.unit.empty() && !quantity.quantity.empty() && quantity.quantity != "dimensionless";
    if (cell.IsValid() && (floating || physical)) {
      double converted = cell.ToDouble();
      std::string unit;
      if (!SimResults::ResultsUnits::display_value(converted, quantity, "report", converted, unit, error)) {
        return false;
      }
      if (std::isfinite(converted)) {
        value = converted;
      }
    } else if (cell.IsValid()) {
      value = QString::fromUtf8(cell.ToString().c_str());
    }
    if (components == 1) {
      output = value;
      return true;
    }
    if (value.isDouble()) {
      text.push_back(QString::number(value.toDouble(), 'g', 17));
    } else {
      text.push_back(value.toString());
    }
  }
  output = "[" + text.join(", ") + "]";
  return true;
}

void unavailable(ReportDocument& document, const std::string& message) {
  ReportDiagnostic diagnostic;
  diagnostic.section_id = "post.results";
  diagnostic.severity = "unavailable";
  diagnostic.message = message;
  document.diagnostics.push_back(std::move(diagnostic));
}

bool collect_tables(const std::vector<std::shared_ptr<vtkTable>>& tables, const std::vector<SimResults::ResultsTableMetadata>& metadata, ReportSection& section, ReportDocument& document, std::string& error) {
  for (std::size_t i = 0; i < tables.size(); ++i) {
    if (!tables[i]) {
      continue;
    }
    if (tables[i]->GetNumberOfRows() > 10000) {
      unavailable(document, "A result table exceeds 10000 report rows; use the Postprocessor table or CSV export for the complete data.");
      continue;
    }
    ReportTable table;
    table.title = "Result table " + std::to_string(i + 1);
    if (i < metadata.size() && !metadata[i].name.empty()) {
      table.title = metadata[i].name;
    }
    std::vector<SimUnits::QuantityUnit> quantities(static_cast<std::size_t>(tables[i]->GetNumberOfColumns()));
    if (i < metadata.size() && metadata[i].columns.size() == quantities.size()) {
      quantities = metadata[i].columns;
    }
    for (vtkIdType column = 0; column < tables[i]->GetNumberOfColumns(); ++column) {
      std::string name = "Column " + std::to_string(column + 1);
      if (tables[i]->GetColumnName(column)) {
        name = tables[i]->GetColumnName(column);
      }
      table.columns.push_back(name);
      double value = 0.0;
      std::string unit;
      if (!SimResults::ResultsUnits::display_value(0.0, quantities[static_cast<std::size_t>(column)], "report", value, unit, error)) {
        return false;
      }
      table.units.push_back(unit);
    }
    for (vtkIdType row = 0; row < tables[i]->GetNumberOfRows(); ++row) {
      std::vector<QJsonValue> values;
      for (vtkIdType column = 0; column < tables[i]->GetNumberOfColumns(); ++column) {
        QJsonValue value;
        if (!report_cell(*tables[i]->GetColumn(column), row, quantities[static_cast<std::size_t>(column)], value, error)) {
          return false;
        }
        values.push_back(std::move(value));
      }
      table.rows.push_back(std::move(values));
    }
    section.tables.push_back(std::move(table));
  }
  return true;
}
}

PostprocessorReportResultProvider::PostprocessorReportResultProvider(std::weak_ptr<PostprocessorApplicationInterface> value) : application(std::move(value)) {
}

bool PostprocessorReportResultProvider::collect(const ReportResultRequest& request, ReportDocument& output, std::string& error) const {
  const auto active = application.lock();
  if (!active) {
    error = "The Postprocessor report provider is unavailable.";
    return false;
  }
  ReportDocument candidate = output;
  std::shared_ptr<const SimResults::VtkResultsData> source;
  if (!PostprocessorReportBlockSummary::source(*active, request.source, source, error))
  {
    return false;
  }
  if (!source) {
    unavailable(candidate, "Open results for this exact solver launch in Postprocessor to include result tables and extrema. Unverified or different runs are not substituted.");
    output = std::move(candidate);
    error.clear();
    return true;
  }
  std::vector<std::size_t> frames;
  if (!PostprocessorReportFrameSelection().select(*source, request.settings, frames, error))
  {
    return false;
  }
  const auto enabled = [&](const std::string& key)
  {
    const auto found = request.settings.values.find(key);
    return found != request.settings.values.end() && found->second == "true";
  };
  ReportSection section;
  section.id = "post.results";
  section.title = "Block results";
  section.paragraphs.push_back("Result source: " + source->result_identity + ". Frame policy: " + request.settings.values.at("post.results.frames") + ". Missing fields are not taken from earlier frames.");
  if (enabled("post.results.extrema"))
  {
    std::vector<PostprocessorReportBlockSummary> summaries;
    if (!PostprocessorReportBlockSummary::collect(request, *source, summaries, error, &candidate.diagnostics))
    {
      return false;
    }
    for (const auto& summary : summaries)
    {
      auto child = summary.section(true);
      if (summary.available)
      {
        const auto& minimum = source->increments[summary.minimum_frame];
        const auto& maximum = source->increments[summary.maximum_frame];
        child.paragraphs.push_back("Minimum: step " + std::to_string(minimum.step) + " / increment " + std::to_string(minimum.increment) +
          "; maximum: step " + std::to_string(maximum.step) + " / increment " + std::to_string(maximum.increment) + ".");
      }
      section.children.push_back(std::move(child));
    }
    if (summaries.empty())
    {
      unavailable(candidate, "No verified block/result component mapping exists for the selected frames.");
    }
  }
  if (enabled("post.results.tables"))
  {
    for (auto index : frames)
    {
      const auto& frame = source->increments[index];
      if (!collect_tables(frame.tables, frame.table_metadata, section, candidate, error))
      {
        return false;
      }
    }
    if (!collect_tables(source->tables, source->table_metadata, section, candidate, error))
    {
      return false;
    }
  }
  if (enabled("post.results.nodal_values") || enabled("post.results.element_values"))
  {
    for (auto index : frames)
    {
      const auto& frame = source->increments[index];
      for (const auto& field : frame.fields)
      {
        if (field.categorical || field.association == SimResults::Association::Table ||
            (field.association == SimResults::Association::Node && !enabled("post.results.nodal_values")) ||
            (field.association != SimResults::Association::Node && !enabled("post.results.element_values")))
        {
          continue;
        }
        for (const auto& part : Postprocessor::result_mesh_parts(frame.dataset.get()))
        {
          if (part.first != field.dataset_id)
          {
            continue;
          }
          for (const auto& component : field.components)
          {
            ReportTable table;
            table.title = field.name + " / dataset " + std::to_string(part.first) + " / source frame " + std::to_string(index + 1);
            table.columns = {"Original entity", "Component", "Value"};
            table.units = {"", "", ""};
            Postprocessor::FieldSelection selection;
            selection.component = component;
            auto raw = field;
            raw.component_units.clear();
            std::vector<double> values;
            if (!Postprocessor::color_values(part.second, raw, selection, values, error))
            {
              return false;
            }
            std::string unit;
            for (std::size_t sample = 0; sample < values.size(); ++sample)
            {
              double value = 0.0;
              if (!SimResults::ResultsUnits::field_value(values[sample], field, component, false,
                    "report", value, unit, error))
              {
                return false;
              }
              if (!std::isfinite(value))
              {
                continue;
              }
              vtkDataArray* ids = part.second->GetPointData()->GetArray("OriginalNodeId");
              if (field.association == SimResults::Association::Element)
              {
                ids = part.second->GetCellData()->GetArray("OriginalElementId");
              }
              else if (field.association == SimResults::Association::IntegrationPoint)
              {
                ids = part.second->GetPointData()->GetArray("OriginalElementId");
              }
              QString entity = "Unavailable";
              if (ids && sample < static_cast<std::size_t>(ids->GetNumberOfTuples()))
              {
                entity = QString::fromStdString(ids->GetVariantValue(static_cast<vtkIdType>(sample)).ToString());
              }
              if (field.association == SimResults::Association::IntegrationPoint)
              {
                auto* points = part.second->GetPointData()->GetArray("IntegrationPointId");
                if (points && sample < static_cast<std::size_t>(points->GetNumberOfTuples()))
                {
                  entity += " / IP " + QString::fromStdString(points->GetVariantValue(static_cast<vtkIdType>(sample)).ToString());
                }
              }
              table.rows.push_back({entity, QString::fromStdString(component), value});
              if (sample % 1024 == 0 && !ReportProgress::advance(request.progress, ReportProgressStage::Results,
                    "Collecting detailed result rows", 0, 0, error))
              {
                return false;
              }
            }
            table.units[2] = unit;
            table.title += " / " + component;
            section.tables.push_back(std::move(table));
          }
        }
      }
    }
  }
  candidate.sections.push_back(std::move(section));
  output = std::move(candidate);
  error.clear();
  return true;
}
