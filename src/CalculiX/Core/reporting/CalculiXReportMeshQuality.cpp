#include <utility>
#include "CalculiXReportMeshQuality.hpp"
#include "CalculiXReportInputReader.hpp"
#include "CalculiX/Core/mesh/CalculiXMeshInterface.hpp"
#include "CalculiX/Core/jobs/CalculiXJobsInterface.hpp"
#include "CalculiX/Core/jobs/CalculiXJobRunRecord.hpp"
#include "Framework/MeshQuality/CubitMeshQualityProvider.hpp"
#include "Framework/MeshQuality/CubitMeshQualityProgress.hpp"
#include "Framework/MeshQuality/MeshQualityDefaults.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QJsonArray>
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <cmath>
#include <set>
namespace
{
  bool capture(const MeshQualitySettings& settings, ReportProgress* progress,
    std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error,
    const std::string& inputFingerprint = {})
  {
    CalculiXMeshInterface mesh;
    std::map<int, std::string> blocks;
    for (int id : mesh.get_blocks())
    {
      blocks.emplace(id, mesh.get_block_name(id));
    }
    return CubitMeshQualityProvider().collect(blocks, framework_units().geometry_unit(), settings, progress, output, error, inputFingerprint);
  }
}
CalculiXReportMeshQuality::CalculiXReportMeshQuality() = default;
bool CalculiXReportMeshQuality::collect(const ReportRequest& request, const ReportSource& source,
  std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error)
{
  MeshQualitySettings settings;
  if (!MeshQualitySettings::decode(request.overrides, settings, error))
  {
    return false;
  }
  if (source.kind == "model")
  {
    return capture(settings, request.progress, output, error);
  }
  if (source.solver_launch_id.empty())
  {
    output.reset();
    error.clear();
    return true;
  }
  std::shared_ptr<const CalculiXJobRunRecord> record;
  if (!CalculiXJobsInterface().get_run_record(source.job_id, record, error) || !record ||
      record->launchId != source.solver_launch_id)
  {
    error = "The quality archive does not match this solver launch.";
    return false;
  }
  if (record->quality && (record->quality->input_fingerprint != record->input.manifestFingerprint ||
      record->quality->coordinate_unit != record->geometryLengthUnit))
  {
    error = "The quality archive is bound to a different exported mesh.";
    return false;
  }
  if (!record->quality)
  {
    auto unavailable = std::make_shared<MeshQualitySnapshot>();
    unavailable->input_fingerprint = record->input.manifestFingerprint;
    unavailable->diagnostics.push_back(record->qualityUnavailableReason);
    if (unavailable->diagnostics.back().empty())
    {
      unavailable->diagnostics.back() = "Mesh quality was not captured for this solver launch.";
    }
    output = std::move(unavailable);
    error.clear();
    return true;
  }
  output = record->quality;
  error.clear();
  return true;
}
bool CalculiXReportMeshQuality::freeze_job(CalculiXJobRunRecord& record, std::string& error)
{
  CubitMeshQualityProgress progress;
  MeshQualitySettings settings;
  // Archive all catalogue metrics once; report inclusion remains independent.
  settings.metrics = MeshQualityDefaults::metrics();
  std::stable_sort(settings.metrics.begin(), settings.metrics.end(), [](const auto& first, const auto& second)
  {
    return first.enabled_by_default && !second.enabled_by_default;
  });
  std::shared_ptr<const MeshQualitySnapshot> captured;
  if (!capture(settings, &progress, captured, error, record.input.manifestFingerprint))
  {
    return false;
  }
  if (record.unitSystem.empty() || record.geometryLengthUnit.empty() ||
      record.geometryLengthUnit != captured->coordinate_unit)
  {
    error = "Cannot bind quality to a job without matching solver and geometry units.";
    return false;
  }
  ReportSource source;
  source.input_files = {record.input.rootFile};
  CalculiXReportInputReader reader;
  if (!reader.read_archive(source, record.input, error, &progress))
  {
    return false;
  }
  std::map<std::int64_t, std::array<double, 3>> nodes;
  std::map<std::int64_t, std::vector<std::int64_t>> elements;
  std::map<std::string, std::set<std::int64_t>> memberships;
  for (const auto& value : reader.records())
  {
    const auto item = value.toObject();
    const auto keyword = item.value("keyword").toString();
    if (keyword != "NODE" && keyword != "ELEMENT")
    {
      continue;
    }
    const auto header = item.value("header").toString();
    const auto match = QRegularExpression("(?:^|,)\\s*ELSET\\s*=\\s*([^,]+)", QRegularExpression::CaseInsensitiveOption).match(header);
    const auto block = match.captured(1).trimmed().toUpper().toStdString();
    std::int64_t pending = 0;
    for (const auto& line : item.value("data").toArray())
    {
      const auto text = line.toString();
      const auto fields = text.split(',');
      bool valid = false;
      if (keyword == "NODE")
      {
        if (fields.size() != 4)
        {
          error = "Unsupported exported node record for quality binding.";
          return false;
        }
        const auto id = fields[0].trimmed().toLongLong(&valid);
        if (!valid || id <= 0)
        {
          error = "Invalid exported node ID for quality binding.";
          return false;
        }
        std::array<double, 3> xyz;
        for (int axis = 0; axis < 3; ++axis)
        {
          const double number = fields[axis + 1].trimmed().toDouble(&valid);
          if (!valid)
          {
            return false;
          }
          double canonical = 0;
          if (!framework_units().system_to_si(number, SimUnits::Length, record.unitSystem, canonical, error) ||
              !framework_units().from_si(canonical, record.geometryLengthUnit, SimUnits::Length,
                SimUnits::UnitRole::Linear, xyz[axis], error))
          {
            return false;
          }
        }
        if (!nodes.emplace(id, xyz).second)
        {
          error = "Repeated exported node in quality binding.";
          return false;
        }
      }
      else
      {
        int start = 0;
        if (pending == 0)
        {
          pending = fields[0].trimmed().toLongLong(&valid);
          if (!valid || pending <= 0 || elements.count(pending) != 0)
          {
            error = "Invalid or repeated exported element in quality binding.";
            return false;
          }
          elements.emplace(pending, std::vector<std::int64_t>{});
          memberships[block].insert(pending);
          start = 1;
        }
        for (int index = start; index < fields.size(); ++index)
        {
          if (fields[index].trimmed().isEmpty())
          {
            continue;
          }
          const auto node = fields[index].trimmed().toLongLong(&valid);
          if (!valid || node <= 0)
          {
            error = "Invalid exported element connectivity in quality binding.";
            return false;
          }
          elements[pending].push_back(node);
        }
        if (!text.trimmed().endsWith(','))
        {
          pending = 0;
        }
      }
    }
    if (pending != 0)
    {
      error = "Unfinished exported element connectivity in quality binding.";
      return false;
    }
  }
  // Verify original identities, membership, ordered topology and coordinates.
  // Do not guess permutations for unsupported exported element formulations.
  for (std::size_t index = 0; index < captured->node_ids.size(); ++index)
  {
    if (index % 2048 == 0 && !progress.update(ReportProgressStage::Model,
          "Verifying archived quality coordinates", index, captured->node_ids.size(), error))
    {
      return false;
    }
    const auto found = nodes.find(captured->node_ids[index]);
    if (found == nodes.end())
    {
      error = "Exported nodes do not match the captured quality mesh.";
      return false;
    }
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      const double original = captured->coordinates[index][axis];
      // Export decimal formatting tolerance, in recorded native geometry units.
      if (!std::isfinite(found->second[axis]) || std::abs(found->second[axis] - original) > 1e-8 * std::max(1.0, std::abs(original)))
      {
        error = "Exported coordinates differ from the captured quality mesh.";
        return false;
      }
    }
  }
  for (const auto& block : captured->blocks)
  {
    const auto name = QString::fromStdString(block.block_name).trimmed().toUpper().toStdString();
    for (std::size_t index = 0; index < block.element_ids.size(); ++index)
    {
      if (index % 2048 == 0 && !progress.update(ReportProgressStage::Model,
            "Verifying archived quality block " + std::to_string(block.block_id), index, block.element_ids.size(), error))
      {
        return false;
      }
      const auto id = block.element_ids[index];
      const auto found = elements.find(id);
      const auto first = block.connectivity.begin() + index * block.nodes_per_element;
      if (found == elements.end() || found->second.size() != block.nodes_per_element ||
          !std::equal(found->second.begin(), found->second.end(), first) || memberships[name].count(id) == 0)
      {
        error = "Exported block membership or node ordering cannot be verified for quality reporting.";
        return false;
      }
    }
  }
  if (!progress.update(ReportProgressStage::Model, "Quality archive prepared", 1, 1, error))
  {
    return false;
  }
  record.quality = std::move(captured);
  record.qualityUnavailableReason.clear();
  error.clear();
  return true;
}
