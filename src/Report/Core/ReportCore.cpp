#include "ReportCore.hpp"
#include <utility>
#include "ReportConfigurationResolver.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportTemplateProviderInterface.hpp"
#include "Framework/Reporting/ReportModelProviderInterface.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include "Framework/Reporting/ReportResultProviderInterface.hpp"
#include "Framework/Reporting/ReportResultRequest.hpp"
#include "Framework/Reporting/ReportImageProviderInterface.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include <QDateTime>
#include <QUuid>
#include <functional>
#include <set>

ReportCore::ReportCore(ReportProviderRegistryInterface& providers) : registry(providers)
{
  state.request.provider = "ccx";
}

ReportCore::~ReportCore() = default;

const ReportSession& ReportCore::session() const noexcept
{
  return state;
}

std::shared_ptr<const ReportDocument> ReportCore::document() const noexcept
{
  const auto found = state.documents.find(state.active_id);
  return found == state.documents.end() ? nullptr : found->second;
}

bool ReportCore::describe(ReportTemplate& output, std::string& error) const
{
  ReportTemplateProviderInterface* templates = nullptr;
  ReportModelProviderInterface* models = nullptr;
  if (!registry.resolve_model_reporting(configuration_request().provider.empty() ? "ccx" : configuration_request().provider, templates, models, error))
  {
    return false;
  }
  ReportTemplate candidate;
  if (!templates->describe(candidate, error))
  {
    return false;
  }
  ReportConfigurationResolver::add_common_options(candidate);
  output = std::move(candidate);
  return true;
}

bool ReportCore::generate(const std::string& provider, int job, const std::string& name, std::string& error)
{
  ReportRequest request;
  request.provider = provider;
  request.job_id = job;
  return generate_request(request, name, error);
}

bool ReportCore::generate_request(const ReportRequest& source, const std::string& name, std::string& error)
{
  ReportRequest request = source;
  ReportTemplateProviderInterface* templates = nullptr;
  ReportModelProviderInterface* models = nullptr;
  ReportTemplate definition;
  if ((request.kind != "job" && request.kind != "model") ||
      (request.kind == "job" && request.job_id <= 0) ||
      (request.kind == "model" && request.model_id.empty()) ||
      !registry.resolve_model_reporting(source.provider, templates, models, error) || !templates->describe(definition, error))
  {
    if (error.empty())
    {
      error = "Report generation requires a valid model or positive job ID.";
    }
    return false;
  }
  const auto& current = configuration_request();
  if (name.empty() && current.provider == source.provider && current.kind == source.kind &&
      current.model_id == source.model_id && current.job_id == source.job_id && !current.overrides.values.empty())
  {
    request.overrides = current.overrides;
    request.profile_id = current.profile_id;
    request.profile_revision = current.profile_revision;
    return build(request, nullptr, false, error);
  }
  if (name.empty())
  {
    if (const auto saved = find_document(source, definition.workspace_id))
    {
      const auto draft = state.drafts.find(saved->id);
      if (draft != state.drafts.end())
      {
        request = draft->second;
      }
      else
      {
        request.overrides = saved->configuration;
        request.profile_id = saved->profile_id;
        request.profile_revision = saved->profile_revision;
      }
      request.provider = source.provider;
      return build(request, nullptr, false, error);
    }
  }
  ReportProfile selected;
  bool found = false;
  if (auto* store = registry.report_profiles())
  {
    if (!name.empty())
    {
      if (!store->load(definition.workspace_id, name, selected, error))
      {
        return false;
      }
      found = true;
    }
    else if (!store->default_profile(definition.workspace_id, selected, found, error))
    {
      return false;
    }
  }
  else if (!name.empty())
  {
    error = "Framework report profiles are unavailable.";
    return false;
  }
  if (found)
  {
    request.overrides = selected.configuration;
    request.profile_id = selected.id;
    request.profile_revision = selected.revision;
  }
  return build(request, found ? &selected : nullptr, false, error);
}

bool ReportCore::build(const ReportRequest& request, const ReportProfile* profile, bool updating, std::string& error)
{
  error.clear();
  ReportTemplateProviderInterface* templates = nullptr;
  ReportModelProviderInterface* models = nullptr;
  ReportTemplate definition;
  if (!registry.resolve_model_reporting(request.provider, templates, models, error) || !templates->describe(definition, error))
  {
    return false;
  }
  std::vector<ReportSourceEntry> liveSources;
  if (!models->available_sources(liveSources, error))
  {
    return false;
  }
  bool sourceAvailable = false;
  for (const auto& entry : liveSources)
  {
    if (entry.request.kind == request.kind &&
        (request.kind == "job" ? entry.request.job_id == request.job_id : entry.request.model_id == request.model_id))
    {
      sourceAvailable = entry.available;
      if (!sourceAvailable)
      {
        error = entry.unavailable_reason;
      }
      break;
    }
  }
  if (!sourceAvailable)
  {
    if (error.empty())
    {
      error = "The selected report source is unavailable for generation or update.";
    }
    return false;
  }
  const auto previous = find_document(request, definition.workspace_id);
  if (!previous && state.documents.size() >= 1000)
  {
    error = "The project already contains the maximum supported number of report sources (1000).";
    return false;
  }
  ReportConfigurationResolver::add_common_options(definition);
  if (profile != nullptr && (profile->template_id != definition.id || profile->template_revision != definition.revision))
  {
    error = "The saved profile requires a different report template revision.";
    return false;
  }
  ReportRequest resolved = request;
  resolved.provider = templates->command_prefix();
  if (!ReportConfigurationResolver::resolve(definition, request.overrides, resolved.overrides, error))
  {
    return false;
  }
  ReportSource source;
  if (!models->resolve_source(resolved, source, error))
  {
    return false;
  }
  auto candidate = std::make_shared<ReportDocument>();
  candidate->id = previous ? previous->id : QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
  candidate->title = resolved.overrides.values.at("title");
  candidate->provider = resolved.provider;
  candidate->template_id = definition.id;
  candidate->template_revision = definition.revision;
  candidate->profile_id = request.profile_id;
  candidate->profile_revision = request.profile_revision;
  candidate->configuration = resolved.overrides;
  candidate->generated_at = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
  if (!models->collect(resolved, source, *candidate, error))
  {
    return false;
  }
  if (candidate->source.workspace_id != definition.workspace_id || candidate->source.kind != request.kind || candidate->source.job_id != request.job_id ||
      (request.kind == "model" && candidate->source.model_id != request.model_id))
  {
    error = "The provider returned report content for a different source.";
    return false;
  }
  if (previous)
  {
    candidate->id = previous->id;
    candidate->notes = previous->notes;
    // User figures are immutable snapshots, independent of generated content.
    for (const auto& section : previous->sections)
    {
      if (section.id == "user.figures")
      {
        candidate->sections.push_back(section);
      }
    }
  }
  if (updating && previous != nullptr)
  {
    if (candidate->source.workspace_id != previous->source.workspace_id ||
        candidate->source.kind != previous->source.kind || candidate->source.model_id != previous->source.model_id ||
        candidate->source.job_id != previous->source.job_id ||
        candidate->source.input_files != previous->source.input_files ||
        (request.kind == "job" && candidate->source.fingerprints != previous->source.fingerprints) ||
        candidate->source.run_identity != previous->source.run_identity ||
        candidate->source.solver_launch_id != previous->source.solver_launch_id ||
        candidate->source.setup_snapshot_id != previous->source.setup_snapshot_id ||
        (request.kind == "job" && candidate->source.snapshot_fingerprint != previous->source.snapshot_fingerprint))
    {
      error = "Report sources changed. Generate again explicitly to replace the report using the current job files.";
      return false;
    }
    candidate->profile_revision = request.profile_revision;
  }
  const auto resultSelection = resolved.overrides.values.find("post.results");
  if (request.kind == "job" && resultSelection != resolved.overrides.values.end() && resultSelection->second == "true")
  {
    if (auto* results = registry.report_results())
    {
      ReportResultRequest resultRequest;
      resultRequest.provider = resolved.provider;
      resultRequest.source = candidate->source;
      resultRequest.settings = resolved.overrides;
      if (!results->collect(resultRequest, *candidate, error))
      {
        return false;
      }
    }
    else
    {
      ReportDiagnostic issue;
      issue.section_id = "post.results";
      issue.severity = "unavailable";
      issue.message = "Result tables, extrema and locations require the Postprocessor reporting provider (Batch B).";
      candidate->diagnostics.push_back(std::move(issue));
    }
  }
  const auto imageSelection = resolved.overrides.values.find("post.images");
  if (auto* images = registry.report_images(resolved.provider))
  {
    ReportImageRequest imageRequest;
    imageRequest.provider = resolved.provider;
    imageRequest.source = candidate->source;
    imageRequest.settings = resolved.overrides;
    if (!images->collect(imageRequest, *candidate, error))
    {
      return false;
    }
  }
  if (imageSelection != resolved.overrides.values.end() && imageSelection->second == "true")
  {
    ReportDiagnostic issue;
    issue.section_id = "post.images";
    issue.severity = "unavailable";
    issue.message = "Result images require the Postprocessor image provider; CalculiX setup images are collected separately.";
    candidate->diagnostics.push_back(std::move(issue));
  }
  if (!candidate->validate(error))
  {
    return false;
  }
  if (state.configuration_changed && !state.active_id.empty())
  {
    state.drafts[state.active_id] = state.request;
  }
  state.drafts.erase(candidate->id);
  state.documents[candidate->id] = candidate;
  state.active_id = candidate->id;
  state.request = std::move(resolved);
  state.configuration_changed = false;
  state.pending_source = false;
  return true;
}

bool ReportCore::update(std::string& error)
{
  if (!document() || state.pending_source)
  {
    error = "Choose a source with a generated report before updating.";
    return false;
  }
  return build(state.request, nullptr, true, error);
}

bool ReportCore::configure(const std::string& key, const std::string& value, std::string& error)
{
  ReportConfiguration patch;
  patch.values[key] = value;
  return configure_patch(patch, error);
}

bool ReportCore::configure_patch(const ReportConfiguration& patch, std::string& error)
{
  ReportTemplate definition;
  if (!describe(definition, error))
  {
    return false;
  }
  auto candidate = configuration_request().overrides;
  if (patch.replace_company_logo)
  {
    candidate.company_logo = patch.company_logo;
  }
  for (const auto& entry : patch.values)
  {
    candidate.values[entry.first] = entry.second;
  }
  ReportConfiguration resolved;
  if (!ReportConfigurationResolver::resolve(definition, candidate, resolved, error))
  {
    return false;
  }
  if (state.pending_source)
  {
    state.pending_request.overrides = std::move(resolved);
  }
  else
  {
    state.request.overrides = std::move(resolved);
    state.configuration_changed = true;
  }
  return true;
}

bool ReportCore::profile(const std::string& action, const std::string& name, std::vector<ReportProfile>& output, std::string& error)
{
  auto* store = registry.report_profiles();
  ReportTemplate definition;
  if (store == nullptr || !describe(definition, error))
  {
    if (error.empty())
    {
      error = "Framework report-profile storage is unavailable.";
    }
    return false;
  }
  if (action == "list")
  {
    return store->list(definition.workspace_id, output, error);
  }
  if (action == "reset")
  {
    ReportProfile profile;
    bool found = false;
    if (!store->default_profile(definition.workspace_id, profile, found, error))
    {
      return false;
    }
    ReportConfiguration values = definition.defaults;
    if (found && (profile.template_id != definition.id || profile.template_revision != definition.revision ||
        !ReportConfigurationResolver::resolve(definition, profile.configuration, values, error)))
    {
      if (error.empty())
      {
        error = "Incompatible workspace default profile.";
      }
      return false;
    }
    auto& target = state.pending_source ? state.pending_request : state.request;
    target.overrides = std::move(values);
    target.profile_id = found ? profile.id : "";
    target.profile_revision = found ? profile.revision : 0;
    if (!state.pending_source)
    {
      state.configuration_changed = true;
    }
    return true;
  }
  if (action == "default")
  {
    ReportProfile profile;
    ReportConfiguration validated;
    if (!store->load(definition.workspace_id, name, profile, error) ||
        profile.template_id != definition.id || profile.template_revision != definition.revision ||
        !ReportConfigurationResolver::resolve(definition, profile.configuration, validated, error))
    {
      if (error.empty())
      {
        error = "Incompatible report profile.";
      }
      return false;
    }
    return store->set_default_profile(definition.workspace_id, name, error);
  }
  if (action == "save")
  {
    ReportProfile profile;
    profile.name = name;
    profile.workspace_id = definition.workspace_id;
    profile.template_id = definition.id;
    profile.template_revision = definition.revision;
    if (!ReportConfigurationResolver::resolve(definition, configuration_request().overrides, profile.configuration, error))
    {
      return false;
    }
    if (!store->save(profile, error))
    {
      return false;
    }
    output = {profile};
    return true;
  }
  if (action == "load")
  {
    ReportProfile profile;
    ReportConfiguration resolved;
    if (!store->load(definition.workspace_id, name, profile, error) ||
        profile.template_id != definition.id || profile.template_revision != definition.revision ||
        !ReportConfigurationResolver::resolve(definition, profile.configuration, resolved, error))
    {
      if (error.empty())
      {
        error = "Incompatible report profile.";
      }
      return false;
    }
    auto& target = state.pending_source ? state.pending_request : state.request;
    target.overrides = std::move(resolved);
    target.profile_id = profile.id;
    target.profile_revision = profile.revision;
    if (!state.pending_source)
    {
      state.configuration_changed = true;
    }
    return true;
  }
  error = "Unknown report profile operation.";
  return false;
}

bool ReportCore::activate_document(const std::string& id, std::string& error)
{
  const auto found = state.documents.find(id);
  if (found == state.documents.end())
  {
    error = "Unknown report ID: " + id;
    return false;
  }
  if (state.configuration_changed && !state.active_id.empty())
  {
    state.drafts[state.active_id] = state.request;
  }
  state.pending_source = false;
  state.active_id = id;
  state.request.provider = found->second->provider;
  state.request.job_id = found->second->source.job_id;
  state.request.kind = found->second->source.kind;
  state.request.model_id = found->second->source.model_id;
  state.request.profile_id = found->second->profile_id;
  state.request.profile_revision = found->second->profile_revision;
  state.request.overrides = found->second->configuration;
  const auto draft = state.drafts.find(id);
  state.configuration_changed = draft != state.drafts.end();
  if (state.configuration_changed)
  {
    state.request = draft->second;
  }
  error.clear();
  return true;
}

bool ReportCore::note(const std::string& section, const std::string& text, std::string& error)
{
  const auto current = document();
  if (!current || text.size() > 65536)
  {
    error = "Select a report and provide a note no longer than 64 KiB.";
    return false;
  }
  std::function<bool(const std::vector<ReportSection>&)> contains = [&](const std::vector<ReportSection>& sections)
  {
    for (const auto& item : sections)
    {
      if (item.id == section || contains(item.children))
      {
        return true;
      }
    }
    return false;
  };
  if (!contains(current->sections))
  {
    error = "The note must target a generated section ID.";
    return false;
  }
  auto candidate = std::make_shared<ReportDocument>(*current);
  candidate->notes[section] = text;
  state.documents[state.active_id] = std::move(candidate);
  error.clear();
  return true;
}

void ReportCore::restore(ReportSession session)
{
  state = std::move(session);
}

void ReportCore::reset()
{
  state = ReportSession();
  state.request.provider = "ccx";
}

const ReportRequest& ReportCore::configuration_request() const noexcept
{
  return state.pending_source ? state.pending_request : state.request;
}

bool ReportCore::available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  std::vector<ReportSourceEntry> entries;
  const bool discovered = registry.available_report_sources(entries, error);
  if (!discovered && state.documents.empty())
  {
    return false;
  }
  const std::string discoveryError = error;
  if (!discovered)
  {
    entries.clear();
  }
  std::set<std::string> represented;
  for (auto& entry : entries)
  {
    if (const auto saved = find_document(entry.request))
    {
      entry.has_report = true;
      represented.insert(saved->id);
    }
  }
  for (const auto& item : state.documents)
  {
    if (represented.count(item.first) != 0)
    {
      continue;
    }
    const auto& saved = *item.second;
    ReportSourceEntry entry;
    entry.request.provider = saved.provider;
    entry.request.kind = saved.source.kind;
    entry.request.model_id = saved.source.model_id;
    entry.request.job_id = saved.source.job_id;
    entry.workspace_label = saved.source.workspace_id;
    entry.label = (saved.source.kind == "model" ? std::string("Current model") : "Job " + std::to_string(saved.source.job_id)) +
      " (saved report; source unavailable)";
    entry.has_report = true;
    entry.unavailable_reason = discoveryError.empty() ? "Saved report can be viewed; its source is unavailable for generation or update." : discoveryError;
    entries.push_back(std::move(entry));
  }
  output = std::move(entries);
  error.clear();
  return true;
}

bool ReportCore::choose_source(const ReportRequest& requested, std::string& error)
{
  ReportRequest source = requested;
  {
    ReportTemplateProviderInterface* templates = nullptr;
    ReportModelProviderInterface* models = nullptr;
    std::string ignored;
    if (registry.resolve_model_reporting(source.provider, templates, models, ignored))
    {
      source.provider = templates->command_prefix();
    }
  }
  std::vector<ReportSourceEntry> entries;
  if (!available_sources(entries, error))
  {
    return false;
  }
  for (const auto& entry : entries)
  {
    if (entry.request.provider != source.provider || entry.request.kind != source.kind ||
        entry.request.job_id != source.job_id || entry.request.model_id != source.model_id)
    {
      continue;
    }
    const auto matches = [&](const ReportRequest& candidate)
    {
      return candidate.provider == source.provider && candidate.kind == source.kind &&
        candidate.model_id == source.model_id && candidate.job_id == source.job_id;
    };
    if (matches(configuration_request()))
    {
      error.clear();
      return true;
    }
    if (const auto saved = find_document(source))
    {
      return activate_document(saved->id, error);
    }
    if (!entry.available)
    {
      error = entry.unavailable_reason;
      return false;
    }
    ReportTemplateProviderInterface* templates = nullptr;
    ReportModelProviderInterface* models = nullptr;
    ReportTemplate definition;
    if (!registry.resolve_model_reporting(source.provider, templates, models, error) || !templates->describe(definition, error))
    {
      return false;
    }
    ReportConfigurationResolver::add_common_options(definition);
    ReportRequest pending = source;
    pending.overrides = definition.defaults;
    if (auto* store = registry.report_profiles())
    {
      ReportProfile profile;
      bool found = false;
      if (!store->default_profile(definition.workspace_id, profile, found, error))
      {
        return false;
      }
      if (found)
      {
        if (profile.template_id != definition.id || profile.template_revision != definition.revision ||
            !ReportConfigurationResolver::resolve(definition, profile.configuration, pending.overrides, error))
        {
          error = "The workspace default profile is incompatible with its template.";
          return false;
        }
        pending.profile_id = profile.id;
        pending.profile_revision = profile.revision;
      }
    }
    state.pending_request = std::move(pending);
    state.pending_source = true;
    error.clear();
    return true;
  }
  error = "The selected report source is no longer available.";
  return false;
}

bool ReportCore::validate_configuration_patch(const ReportConfiguration& patch, std::string& error) const
{
  ReportTemplate definition;
  if (!describe(definition, error))
  {
    return false;
  }
  auto candidate = configuration_request().overrides;
  if (patch.replace_company_logo)
  {
    candidate.company_logo = patch.company_logo;
  }
  for (const auto& entry : patch.values)
  {
    candidate.values[entry.first] = entry.second;
  }
  ReportConfiguration resolved;
  return ReportConfigurationResolver::resolve(definition, candidate, resolved, error);
}

std::shared_ptr<const ReportDocument> ReportCore::find_document(const ReportRequest& source, const std::string& workspace) const
{
  std::string owner = workspace;
  if (owner.empty())
  {
    ReportTemplateProviderInterface* templates = nullptr;
    ReportModelProviderInterface* models = nullptr;
    ReportTemplate definition;
    std::string ignored;
    if (registry.resolve_model_reporting(source.provider, templates, models, ignored) && templates->describe(definition, ignored))
    {
      owner = definition.workspace_id;
    }
  }
  // Source identity excludes title, profile, input fingerprints and run identity.
  // Those may change when Generate replaces the report for the same model/job.
  for (const auto& item : state.documents)
  {
    const auto& saved = *item.second;
    if ((owner.empty() ? saved.provider == source.provider : saved.source.workspace_id == owner) &&
        saved.source.kind == source.kind &&
        (source.kind == "job" ? saved.source.job_id == source.job_id : saved.source.model_id == source.model_id))
    {
      return item.second;
    }
  }
  return nullptr;
}
