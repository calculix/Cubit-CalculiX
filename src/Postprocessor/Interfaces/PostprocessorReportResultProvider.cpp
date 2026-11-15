#include "PostprocessorReportResultProvider.hpp"
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

bool entity_position(vtkDataSet* mesh, const SimResults::Field& field, std::size_t sample, const SimUnits::QuantityUnit& quantity, std::array<QJsonValue, 3>& output, std::string& unit, std::string& error) {
  output.fill(QStringLiteral("Unavailable"));
  double unused = 0.0;
  if (!SimResults::ResultsUnits::display_value(0.0, quantity, "report", unused, unit, error)) {
    return false;
  }
  if (!mesh) {
    return true;
  }
  double position[3] = {0.0, 0.0, 0.0};
  if (field.association == SimResults::Association::Element) {
    if (sample >= static_cast<std::size_t>(mesh->GetNumberOfCells())) {
      return true;
    }
    auto* cell = mesh->GetCell(static_cast<vtkIdType>(sample));
    if (!cell || cell->GetNumberOfPoints() == 0) {
      return true;
    }
    double coordinates[3] = {0.0, 0.0, 0.0};
    int subId = cell->GetParametricCenter(coordinates);
    std::vector<double> weights(static_cast<std::size_t>(cell->GetNumberOfPoints()));
    cell->EvaluateLocation(subId, coordinates, position, weights.data());
  } else {
    if (sample >= static_cast<std::size_t>(mesh->GetNumberOfPoints())) {
      return true;
    }
    mesh->GetPoint(static_cast<vtkIdType>(sample), position);
  }
  for (std::size_t axis = 0; axis < output.size(); ++axis) {
    double converted = 0.0;
    if (!SimResults::ResultsUnits::display_value(position[axis], quantity, "report", converted, unit, error)) {
      return false;
    }
    if (std::isfinite(converted)) {
      output[axis] = converted;
    }
  }
  return true;
}

std::string entity_id(vtkDataSet& mesh, const SimResults::Field& field, std::size_t sample) {
  vtkDataArray* ids = nullptr;
  if (field.association == SimResults::Association::Element) {
    ids = mesh.GetCellData()->GetArray("OriginalElementId");
  } else if (field.association == SimResults::Association::Node) {
    ids = mesh.GetPointData()->GetArray("OriginalNodeId");
  } else {
    ids = mesh.GetPointData()->GetArray("OriginalElementId");
  }
  if (!ids || sample >= static_cast<std::size_t>(ids->GetNumberOfTuples())) {
    return "unavailable";
  }
  std::string result = ids->GetVariantValue(static_cast<vtkIdType>(sample)).ToString();
  if (field.association == SimResults::Association::IntegrationPoint) {
    auto* ip = mesh.GetPointData()->GetArray("IntegrationPointId");
    if (ip && sample < static_cast<std::size_t>(ip->GetNumberOfTuples())) {
      result += " / IP " + std::string(ip->GetVariantValue(static_cast<vtkIdType>(sample)).ToString());
    }
  }
  return result;
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
  for (const auto& entry : active->session().sources) {
    const auto data = active->result_data(entry.first);
    if (!data || data->job_id != request.source.job_id || data->source_workspace != request.source.workspace_id) {
      continue;
    }
    if (request.source.solver_launch_id.empty() || data->solver_launch_id != request.source.solver_launch_id) {
      continue;
    }
    if (source && source->result_identity != data->result_identity) {
      error = "Several retained result sources match this solver launch; close the ambiguous source before reporting.";
      return false;
    }
    source = data;
  }
  if (!source) {
    unavailable(candidate, "Open results for this exact solver launch in Postprocessor to include result tables and extrema. Unverified or different runs are not substituted.");
    output = std::move(candidate);
    error.clear();
    return true;
  }
  ReportSection section;
  section.id = "post.results";
  section.title = "Result tables and extrema";
  section.paragraphs.push_back("Result source: " + source->result_identity + ". Numeric source arrays remain unchanged; values use Report presentation units. Extrema positions are undeformed node or integration-point coordinates; element values use the element parametric center.");
  std::map<std::string, ReportTable> extrema;
  for (const auto& frame : source->increments) {
    SimUnits::QuantityUnit coordinates("length", frame.coordinate_unit, SimUnits::Length);
    if (coordinates.unit.empty()) {
      coordinates.unit = source->source_units.coordinate_unit;
    }
    for (const auto& field : frame.fields) {
      if (field.categorical || field.association == SimResults::Association::Table) {
        continue;
      }
      for (std::size_t component = 0; component < field.components.size(); ++component) {
        auto raw = field;
        raw.component_units.clear();
        Postprocessor::FieldSelection selection;
        selection.component = field.components[component];
        double minimum = std::numeric_limits<double>::infinity();
        double maximum = -std::numeric_limits<double>::infinity();
        std::string minimumId;
        std::string maximumId;
        vtkDataSet* minimumMesh = nullptr;
        vtkDataSet* maximumMesh = nullptr;
        std::size_t minimumSample = 0;
        std::size_t maximumSample = 0;
        for (const auto& part : Postprocessor::result_mesh_parts(frame.dataset.get())) {
          if (part.first != field.dataset_id) {
            continue;
          }
          std::vector<double> values;
          if (!Postprocessor::color_values(part.second, raw, selection, values, error)) {
            return false;
          }
          for (std::size_t sample = 0; sample < values.size(); ++sample) {
            const double value = values[sample];
            if (!std::isfinite(value)) {
              continue;
            }
            if (value < minimum) {
              minimum = value;
              minimumId = entity_id(*part.second, field, sample);
              minimumMesh = part.second;
              minimumSample = sample;
            }
            if (value > maximum) {
              maximum = value;
              maximumId = entity_id(*part.second, field, sample);
              maximumMesh = part.second;
              maximumSample = sample;
            }
          }
        }
        if (!std::isfinite(minimum)) {
          continue;
        }
        std::string unit;
        if (!SimResults::ResultsUnits::field_value(minimum, field, selection.component, false, "report", minimum, unit, error) || !SimResults::ResultsUnits::field_value(maximum, field, selection.component, false, "report", maximum, unit, error)) {
          return false;
        }
        double parameter = frame.time;
        std::string parameterUnit;
        if (!SimResults::ResultsUnits::display_value(frame.time, frame.independent_variable, "report", parameter, parameterUnit, error)) {
          return false;
        }
        std::array<QJsonValue, 3> minimumPosition;
        std::array<QJsonValue, 3> maximumPosition;
        std::string coordinateUnit;
        if (!entity_position(minimumMesh, field, minimumSample, coordinates, minimumPosition, coordinateUnit, error) || !entity_position(maximumMesh, field, maximumSample, coordinates, maximumPosition, coordinateUnit, error)) {
          return false;
        }
        const auto key = Postprocessor::field_group_key(field) + "/" + std::to_string(field.dataset_id) + "/" + std::to_string(component) + "/" + frame.independent_variable.quantity + "/" + frame.independent_variable.unit;
        auto& table = extrema[key];
        table.title = field.name + " / " + selection.component + " / Dataset " + std::to_string(field.dataset_id);
        table.columns = {"Step", "Increment", "Analysis parameter", "Minimum", "Minimum entity", "Minimum X", "Minimum Y", "Minimum Z", "Maximum", "Maximum entity", "Maximum X", "Maximum Y", "Maximum Z"};
        table.units = {"", "", parameterUnit, unit, "", coordinateUnit, coordinateUnit, coordinateUnit, unit, "", coordinateUnit, coordinateUnit, coordinateUnit};
        table.rows.push_back({frame.step, frame.increment, parameter, minimum, QString::fromUtf8(minimumId.c_str()), minimumPosition[0], minimumPosition[1], minimumPosition[2], maximum, QString::fromUtf8(maximumId.c_str()), maximumPosition[0], maximumPosition[1], maximumPosition[2]});
      }
    }
    if (!collect_tables(frame.tables, frame.table_metadata, section, candidate, error)) {
      return false;
    }
  }
  for (auto& entry : extrema) {
    section.tables.push_back(std::move(entry.second));
  }
  if (!collect_tables(source->tables, source->table_metadata, section, candidate, error)) {
    return false;
  }
  candidate.sections.push_back(std::move(section));
  output = std::move(candidate);
  error.clear();
  return true;
}
