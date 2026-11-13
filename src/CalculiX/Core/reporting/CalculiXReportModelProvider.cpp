#include "CalculiXReportModelProvider.hpp"
#include "CalculiX/Core/jobs/CalculiXJobRunRecord.hpp"
#include "CalculiXReportModelSnapshot.hpp"
#include "Framework/Reporting/ReportSourceEntry.hpp"
#include <utility>
#include "CalculiXReportInputReader.hpp"
#include "CalculiXReportTemplateProvider.hpp"
#include "CalculiX/Core/jobs/CalculiXJobsInterface.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportRequest.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <map>
#include <set>

namespace
{
bool enabled(const ReportRequest& request, const std::string& id)
{
  const auto entry = request.overrides.values.find(id);
  return entry != request.overrides.values.end() && entry->second == "true";
}

QString parameter(const QString& header, const QString& key)
{
  const auto match = QRegularExpression("(?:^|,)\\s*" + key + "\\s*=\\s*([^,]+)", QRegularExpression::CaseInsensitiveOption).match(header);
  return match.captured(1).trimmed();
}

std::string category(const QString& keyword)
{
  if (keyword == "NODE" || keyword == "ELEMENT")
  {
    return "ccx.model.mesh";
  }
  if (keyword == "NSET" || keyword == "ELSET" || keyword == "SURFACE")
  {
    return "ccx.model.sets";
  }
  if (keyword.contains("SECTION") || keyword == "ORIENTATION")
  {
    return "ccx.model.sections";
  }
  if (keyword.contains("INTERACTION") || keyword.contains("CONTACT") || keyword == "FRICTION" ||
      keyword.contains("SURFACE BEHAVIOR") || keyword == "TIE" || keyword == "EQUATION" || keyword == "RIGID BODY" || keyword == "MPC")
  {
    return "ccx.model.interactions";
  }
  if (keyword == "INITIAL CONDITIONS")
  {
    return "ccx.model.initial_conditions";
  }
  return "ccx.other";
}

ReportTable recordTable(const QJsonObject& record)
{
  ReportTable table;
  table.title = record.value("header").toString().toStdString();
  table.columns = {"Record", "Input values"};
  int i = 0;
  for (const auto value : record.value("data").toArray())
  {
    table.rows.push_back({++i, value.toString()});
  }
  if (table.rows.empty())
  {
    table.rows.push_back({"Definition", "No data lines"});
  }
  return table;
}
}

CalculiXReportModelProvider::CalculiXReportModelProvider() = default;
CalculiXReportModelProvider::~CalculiXReportModelProvider() = default;

bool CalculiXReportModelProvider::resolve_source(const ReportRequest& request, ReportSource& output, std::string& error) const
{
  if (request.kind == "model")
  {
    if (request.model_id != "current")
    {
      error = "CalculiX reporting supports the current project model only.";
      return false;
    }
    ReportSource source;
    source.kind = "model";
    source.model_id = "current";
    source.workspace_id = "calculix";
    source.provenance = "Current CalculiX model captured through read-only Core interfaces; no solver run is associated.";
    output = std::move(source);
    error.clear();
    return true;
  }
  return CalculiXJobsInterface().get_report_source(request.job_id, output, error);
}

bool CalculiXReportModelProvider::collect(const ReportRequest& request, const ReportSource& source,
  ReportDocument& output, std::string& error) const
{
  if (request.kind == "model")
  {
    return CalculiXReportModelSnapshot().collect(request, source, output, error);
  }
  CalculiXReportInputReader reader;
  bool inputReady = false;
  if (!source.solver_launch_id.empty())
  {
    std::shared_ptr<const CalculiXJobRunRecord> record;
    if (!CalculiXJobsInterface().get_run_record(source.job_id, record, error) || record->launchId != source.solver_launch_id)
    {
      error = "The requested archived solver launch is no longer the job's latest launch.";
      return false;
    }
    inputReady = reader.read_archive(source, record->input, error);
  }
  else
  {
    inputReady = reader.read(source, error);
  }
  if (!inputReady)
  {
    return false;
  }
  ReportTemplate definition;
  if (!CalculiXReportTemplateProvider().describe(definition, error))
  {
    return false;
  }
  ReportDocument candidate = output;
  candidate.source = reader.resolved_source();
  std::map<std::string, ReportSection> sections;
  for (const auto& section : definition.sections)
  {
    if (enabled(request, section.id) && section.id.rfind("post.", 0) != 0)
    {
      sections.emplace(section.id, section);
    }
  }
  std::map<int, ReportSection> steps;
  std::set<int> requestedSteps;
  const auto selection = request.overrides.values.find("steps");
  if (selection != request.overrides.values.end() && selection->second != "all")
  {
    for (const auto& item : QString::fromStdString(selection->second).split(','))
    {
      requestedSteps.insert(item.toInt());
    }
  }
  std::set<int> nodeIds, elementIds;
  std::map<std::string, std::pair<QString, int>> elementBlocks;
  bool material = false;
  int currentStep = 0;
  std::set<int> availableSteps;
  std::map<QString, QString> activeBoundaries;
  for (const auto entry : reader.records())
  {
    const auto record = entry.toObject();
    const auto keyword = record.value("keyword").toString();
    const auto header = record.value("header").toString();
    if (keyword == "STEP")
    {
      currentStep = record.value("step").toInt();
      availableSteps.insert(currentStep);
      ReportSection step;
      step.id = "ccx.steps." + std::to_string(currentStep);
      step.title = "Step " + std::to_string(currentStep) + " - " + record.value("step_name").toString().toStdString();
      steps[currentStep] = step;
      material = false;
    }
    if (currentStep > 0)
    {
      if (keyword == "BOUNDARY")
      {
        if (parameter(header, "OP").compare("NEW", Qt::CaseInsensitive) == 0)
        {
          activeBoundaries.clear();
        }
        for (const auto value : record.value("data").toArray())
        {
          const auto parts = value.toString().split(',');
          bool firstOk = false, lastOk = false;
          const int first = parts.value(1).trimmed().toInt(&firstOk);
          const int last = parts.value(2).trimmed().toInt(&lastOk);
          if (!firstOk || (!parts.value(2).trimmed().isEmpty() && !lastOk) || first < 1 || first > 11 || (lastOk && (last < first || last > 11)))
          {
            ReportDiagnostic issue;
            issue.section_id = steps[currentStep].id;
            issue.severity = "warning";
            issue.message = "A boundary record could not be expanded into effective DOFs; consult its raw definition.";
            candidate.diagnostics.push_back(issue);
            continue;
          }
          for (int dof = first; dof <= (lastOk ? last : first); ++dof)
          {
            activeBoundaries[parts.value(0).trimmed() + "/" + QString::number(dof)] = value.toString();
          }
        }
      }
      if (keyword == "END STEP")
      {
        if (enabled(request, "ccx.steps.boundary_conditions"))
        {
          ReportTable effective;
          effective.title = "Accumulated boundary records (set aliases/overlap not resolved)";
          effective.columns = {"Target / DOF", "Latest input definition"};
          for (const auto& item : activeBoundaries)
          {
            effective.rows.push_back({item.first, item.second});
          }
          steps[currentStep].tables.push_back(std::move(effective));
          steps[currentStep].paragraphs.push_back("The table propagates literal target/DOF records and OP=NEW. Overlapping node sets, transformed coordinates and unsupported boundary options require solver-specific interpretation; raw records remain authoritative.");
        }
        currentStep = 0;
        continue;
      }
      std::string setting = "parameters";
      if (keyword == "BOUNDARY")
      {
        setting = "boundary_conditions";
      }
      else if (keyword.endsWith("FILE") || keyword.endsWith("OUTPUT"))
      {
        setting = "field_output";
      }
      else if (keyword.endsWith("PRINT"))
      {
        setting = "history_output";
      }
      else if (keyword == "MODEL CHANGE")
      {
        setting = "model_changes";
      }
      else if (keyword == "CLOAD" || keyword == "DLOAD" || keyword == "TEMPERATURE" || keyword == "DFLUX" || keyword == "CFLUX" || keyword == "FILM" || keyword == "RADIATE")
      {
        setting = "loads";
      }
      if (enabled(request, "ccx.steps." + setting))
      {
        steps[currentStep].tables.push_back(recordTable(record));
      }
      continue;
    }
    if (keyword == "NODE" || keyword == "ELEMENT")
    {
      int count = 0;
      for (const auto value : record.value("data").toArray())
      {
        bool ok = false;
        const int id = value.toString().section(',', 0, 0).trimmed().toInt(&ok);
        if (ok && id > 0)
        {
          (keyword == "NODE" ? nodeIds : elementIds).insert(id);
          ++count;
        }
      }
      if (keyword == "ELEMENT")
      {
        auto& block = elementBlocks[parameter(header, "ELSET").toStdString()];
        block.first = parameter(header, "TYPE");
        block.second += count;
      }
      material = false;
      continue;
    }
    if (keyword == "MATERIAL")
    {
      material = true;
    }
    const auto group = category(keyword);
    if (group != "ccx.other")
    {
      material = false;
    }
    const std::set<QString> materialKeywords = {"MATERIAL", "ELASTIC", "PLASTIC", "DENSITY", "EXPANSION",
      "CONDUCTIVITY", "SPECIFIC HEAT", "CREEP", "HYPERELASTIC", "HYPERFOAM", "DEFORMATION PLASTICITY",
      "USER MATERIAL", "DEPVAR", "DAMPING", "CYCLIC HARDENING", "GAP CONDUCTANCE"};
    const std::string target = material && materialKeywords.count(keyword) != 0 ? "ccx.model.materials" : group;
    if (sections.count(target) != 0)
    {
      sections[target].tables.push_back(recordTable(record));
    }
    if (keyword == "AMPLITUDE" && enabled(request, "ccx.steps.amplitudes") && sections.count("ccx.steps") != 0)
    {
      sections["ccx.steps"].tables.push_back(recordTable(record));
    }
  }
  for (int wanted : requestedSteps)
  {
    if (availableSteps.count(wanted) == 0)
    {
      error = "Selected CalculiX step does not exist: " + std::to_string(wanted);
      return false;
    }
  }
  if (sections.count("ccx.steps") != 0)
  {
    for (const auto& step : steps)
    {
      if (requestedSteps.empty() || requestedSteps.count(step.first) != 0)
      {
        sections["ccx.steps"].children.push_back(step.second);
      }
    }
  }
  ReportTable summary;
  summary.title = "Input mesh inventory";
  summary.columns = {"Property", "Value"};
  summary.rows = {{"Nodes", static_cast<int>(nodeIds.size())}, {"Elements", static_cast<int>(elementIds.size())}, {"Analysis steps", static_cast<int>(steps.size())}};
  if (sections.count("ccx.model.overview") != 0)
  {
    sections["ccx.model.overview"].tables.push_back(summary);
    sections["ccx.model.overview"].paragraphs.push_back("This report reads the existing job input deck. It does not regenerate the deck or assert that it matches a historical solver run. Units are not inferred.");
  }
  if (sections.count("ccx.model.mesh") != 0)
  {
    sections["ccx.model.mesh"].tables.push_back(summary);
    sections["ccx.model.mesh"].paragraphs.push_back("Mesh inventory counts explicit NODE/ELEMENT records; generated, external mesh or continuation conventions may require additional provider support. Full connectivity is not printed.");
  }
  if (sections.count("ccx.model.blocks") != 0)
  {
    ReportTable table;
    table.title = "Element blocks declared on ELEMENT records";
    table.columns = {"Element set", "Element type", "Explicit element records"};
    for (const auto& block : elementBlocks)
    {
      table.rows.push_back({QString::fromStdString(block.first), block.second.first, block.second.second});
    }
    sections["ccx.model.blocks"].tables.push_back(std::move(table));
  }
  candidate.sections.clear();
  for (const auto& item : definition.sections)
  {
    if (sections.count(item.id) != 0)
    {
      candidate.sections.push_back(std::move(sections[item.id]));
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}

bool CalculiXReportModelProvider::available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  std::vector<ReportSourceEntry> entries;
  ReportSourceEntry model;
  model.request.provider = "ccx";
  model.request.kind = "model";
  model.request.model_id = "current";
  model.workspace_label = "CalculiX";
  model.label = "Current model";
  model.available = true;
  entries.push_back(model);
  CalculiXJobsInterface jobs;
  for (const auto& row : jobs.get_jobs_tree_data())
  {
    if (row.size() < 2)
    {
      error = "CalculiX returned an incomplete job entry.";
      return false;
    }
    bool ok = false;
    const int id = QString::fromStdString(row[0]).toInt(&ok);
    if (!ok || id <= 0)
    {
      error = "CalculiX returned an invalid job ID.";
      return false;
    }
    ReportSourceEntry job;
    job.request.provider = "ccx";
    job.request.job_id = id;
    job.workspace_label = "CalculiX";
    job.label = "Job " + std::to_string(id) + " - " + row[1];
    ReportSource source;
    job.available = jobs.get_report_source(id, source, job.unavailable_reason);
    entries.push_back(std::move(job));
  }
  output = std::move(entries);
  error.clear();
  return true;
}
