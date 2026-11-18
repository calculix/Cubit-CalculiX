#include <utility>
#include "ReportMeshQualityContent.hpp"
#include "Framework/MeshQuality/MeshQualityPresentation.hpp"
#include "Framework/Core/units/QuantityUnit.hpp"
#include <QVariant>
#include "Framework/MeshQuality/MeshQualityStatistics.hpp"
#include <cmath>
#include <iterator>
#include <QString>
ReportMeshQualityContent::ReportMeshQualityContent() = default;
bool ReportMeshQualityContent::append(const MeshQualitySnapshot& snapshot, const MeshQualitySettings& settings,
  ReportProgress* progress, ReportDocument& output, std::string& error)
{
  std::vector<ReportDiagnostic> issues;
  ReportSection root;
  root.id = "mesh.quality";
  root.title = "Mesh quality";
  root.paragraphs = {"Evaluator: " + snapshot.evaluator_version, "Evaluated mesh SHA-256: " + snapshot.mesh_fingerprint,
    "Acceptance limits are editable reporting settings. Standard deviation is population standard deviation; failing percentages use evaluated elements."};
  if (snapshot.mesh_fingerprint.empty())
  {
    root.paragraphs = {"Native mesh quality data is unavailable for this source. See the diagnostics."};
  }
  for (const auto& issue : snapshot.diagnostics)
  {
    issues.push_back({"mesh.quality", "unavailable", issue});
  }
  std::size_t completed = 0;
  for (const auto& block : snapshot.blocks)
  {
    ReportSection section;
    section.id = "mesh.quality.block." + std::to_string(block.block_id) + "." + block.family + "." + std::to_string(block.nodes_per_element);
    section.title = "Block " + std::to_string(block.block_id) + " - " + block.block_name + " / " + block.family +
      " / " + std::to_string(block.nodes_per_element) + " nodes per element";
    section.paragraphs.push_back("Original undeformed mesh. Cubit native metric definitions apply; higher-order support is metric-specific. Images show corner topology.");
    for (const auto& configured : settings.metrics)
    {
      if (configured.family != block.family)
      {
        continue;
      }
      if (!ReportProgress::advance(progress, ReportProgressStage::Model,
            "Summarizing quality / block " + std::to_string(block.block_id) + " / " + configured.label,
            completed, snapshot.blocks.size(), error))
      {
        return false;
      }
      const auto measured = block.values.find(configured.id);
      if (measured == block.values.end())
      {
        std::string reason = "Metric was not captured for this mesh.";
        const auto issue = block.unavailable.find(configured.id);
        if (issue != block.unavailable.end())
        {
          reason = issue->second;
        }
        issues.push_back({section.id, "unavailable", configured.label + ": " + reason});
        continue;
      }
      if (configured.quantity != "dimensionless" && snapshot.coordinate_unit.empty())
      {
        issues.emplace_back(section.id, "unavailable", configured.label + ": native geometry units are unknown.");
        continue;
      }
      MeshQualityPresentation presentation;
      if (!presentation.prepare(configured, snapshot.coordinate_unit, output.configuration, measured->second,
            progress, ReportProgressStage::Model, error))
      {
        return false;
      }
      const auto& metric = presentation.metric;
      const auto& samples = presentation.values(measured->second);
      MeshQualityStatistics statistics;
      if (!statistics.calculate(block.element_ids, samples, metric, error))
      {
        return false;
      }
      const bool assessed = metric.minimum || metric.maximum;
      const auto value = [&](double number)
      {
        if (statistics.evaluated != 0)
        {
          return QJsonValue(number);
        }
        return QJsonValue("");
      };
      const auto limit = [](const std::optional<double>& number)
      {
        if (number)
        {
          return QJsonValue(*number);
        }
        return QJsonValue("");
      };
      section.paragraphs.push_back(metric.label + " / " + metric.unit + " / acceptable minimum: " +
        limit(metric.minimum).toVariant().toString().toStdString() + " / maximum: " +
        limit(metric.maximum).toVariant().toString().toStdString());
      if (settings.statistics)
      {
        ReportTable table;
        table.title = metric.label;
        table.columns = {"Total", "Evaluated", "Unevaluable", "Minimum", "Minimum element", "Maximum", "Maximum element",
          "Mean", "Std. deviation", "Acceptable minimum", "Acceptable maximum", "Failing", "Failing %"};
        QJsonValue minimumId(""), maximumId(""), failed(""), percentage("");
        if (statistics.evaluated != 0)
        {
          minimumId = QString::number(qlonglong(statistics.minimum_id));
          maximumId = QString::number(qlonglong(statistics.maximum_id));
          if (assessed)
          {
            failed = static_cast<double>(statistics.failed);
            percentage = 100.0 * statistics.failed / statistics.evaluated;
          }
        }
        table.rows.push_back({static_cast<double>(block.element_ids.size()), static_cast<double>(statistics.evaluated),
          static_cast<double>(statistics.unevaluable), value(statistics.minimum), minimumId, value(statistics.maximum), maximumId,
          value(statistics.mean), value(statistics.standard_deviation), limit(metric.minimum), limit(metric.maximum), failed, percentage});
        table.units.resize(table.columns.size());
        table.column_quantities.resize(table.columns.size());
        const auto& quantity = presentation.quantity;
        for (std::size_t column : {3u, 5u, 7u, 8u, 9u, 10u})
        {
          table.units[column] = metric.unit;
          table.column_quantities[column] = ReportQuantity(quantity);
        }
        section.tables.push_back(std::move(table));
      }
      if (settings.element_values)
      {
        ReportTable table;
        table.title = metric.label + " - detailed element values";
        table.columns = {"Element ID", "Value", "Assessment"};
        for (std::size_t index = 0; index < block.element_ids.size(); ++index)
        {
          if (index % 2048 == 0 && !ReportProgress::advance(progress, ReportProgressStage::Model,
                "Listing quality elements / " + metric.label, index, block.element_ids.size(), error))
          {
            return false;
          }
          const double sample = samples[index];
          QJsonValue number("");
          std::string status = "Unevaluable";
          if (std::isfinite(sample))
          {
            number = sample;
            status = "Not assessed";
            if (assessed)
            {
              status = "Pass";
              if (MeshQualityStatistics::outside(sample, metric))
              {
                status = "Fail";
              }
            }
          }
          table.rows.push_back({QString::number(qlonglong(block.element_ids[index])), number, QString::fromStdString(status)});
        }
        table.units = {"", metric.unit, ""};
        table.column_quantities.resize(3);
        table.column_quantities[1] = ReportQuantity(presentation.quantity);
        section.tables.push_back(std::move(table));
      }
    }
    root.children.push_back(std::move(section));
    ++completed;
  }
  if (snapshot.blocks.empty() && snapshot.diagnostics.empty())
  {
    issues.push_back({"mesh.quality", "unavailable", "No supported mesh blocks were available."});
  }
  output.sections.push_back(std::move(root));
  output.diagnostics.insert(output.diagnostics.end(), std::make_move_iterator(issues.begin()), std::make_move_iterator(issues.end()));
  error.clear();
  return true;
}
