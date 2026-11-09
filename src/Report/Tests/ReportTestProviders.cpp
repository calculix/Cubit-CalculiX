#include "ReportTestProviders.hpp"
#include "Framework/Reporting/ReportSourceEntry.hpp"
#include <utility>
#include "Framework/Reporting/ReportTemplate.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportRequest.hpp"

ReportTestProviders::ReportTestProviders() = default;
ReportTestProviders::~ReportTestProviders() = default;

bool ReportTestProviders::resolve_model_reporting(const std::string& prefix, ReportTemplateProviderInterface*& templates,
  ReportModelProviderInterface*& models, std::string& error) const
{
  if (prefix != "test")
  {
    error = "Unknown test provider.";
    return false;
  }
  templates = const_cast<ReportTestProviders*>(this);
  models = const_cast<ReportTestProviders*>(this);
  error.clear();
  return true;
}

ReportProfileStoreInterface* ReportTestProviders::report_profiles() const noexcept
{
  return profiles;
}

ReportResultProviderInterface* ReportTestProviders::report_results() const noexcept
{
  return nullptr;
}

ReportImageProviderInterface* ReportTestProviders::report_images(const std::string&) const noexcept
{
  return nullptr;
}

std::string ReportTestProviders::command_prefix() const
{
  return "test";
}

bool ReportTestProviders::describe(ReportTemplate& output, std::string& error) const
{
  ReportTemplate result;
  result.id = "test.standard";
  result.workspace_id = "test";
  ReportOption option;
  option.key = "test.content";
  option.kind = "boolean";
  option.default_value = "true";
  result.options.push_back(option);
  result.defaults.values[option.key] = option.default_value;
  output = std::move(result);
  error.clear();
  return true;
}

bool ReportTestProviders::resolve_source(const ReportRequest& request, ReportSource& output, std::string& error) const
{
  ReportSource source;
  source.workspace_id = "test";
  source.job_id = request.job_id;
  source.kind = request.kind;
  source.model_id = request.model_id;
  source.input_files = {"fixture.inp"};
  source.fingerprints = {std::string(64, changed_source ? '2' : '1')};
  if (request.kind == "model")
  {
    source.input_files.clear();
    source.fingerprints.clear();
    source.snapshot_fingerprint = std::string(64, changed_source ? '2' : '1');
  }
  source.provenance = "Test fixture";
  output = std::move(source);
  error.clear();
  return true;
}

bool ReportTestProviders::collect(const ReportRequest&, const ReportSource& source, ReportDocument& output, std::string& error) const
{
  if (fail_collection)
  {
    error = "Requested provider failure.";
    return false;
  }
  auto candidate = output;
  candidate.source = source;
  ReportSection section;
  section.id = "test.content";
  section.title = "Numbers and escaping";
  section.paragraphs = {"Literal <tag> & text"};
  ReportTable table;
  table.title = "Numeric table";
  table.columns = {"Name", "Value"};
  table.rows = {{"Value", 1.25}};
  section.tables.push_back(std::move(table));
  candidate.sections = {section};
  output = std::move(candidate);
  error.clear();
  return true;
}

bool ReportTestProviders::available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  if (fail_discovery)
  {
    error = "Requested discovery failure.";
    return false;
  }
  std::vector<ReportSourceEntry> entries;
  for (int job : {0, 1, 2})
  {
    if (!publish_sources)
    {
      break;
    }
    ReportSourceEntry entry;
    entry.request.provider = "test";
    entry.request.kind = job == 0 ? "model" : "job";
    entry.request.model_id = job == 0 ? "current" : "";
    entry.request.job_id = job;
    entry.workspace_label = "Test";
    entry.label = job == 0 ? "Current model" : "Job " + std::to_string(job);
    entry.available = true;
    entries.push_back(entry);
  }
  output = std::move(entries);
  error.clear();
  return true;
}

bool ReportTestProviders::available_report_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  return available_sources(output, error);
}
