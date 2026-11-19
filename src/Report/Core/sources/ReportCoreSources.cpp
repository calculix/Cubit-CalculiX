#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportTemplateProviderInterface.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include <set>
#include <utility>

const ReportRequest& ReportCore::configuration_request() const noexcept
{
  if (state.pending_source) {
    return state.pending_request;
  }
  return state.request;
}

bool ReportCore::available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  std::vector<ReportSourceEntry> entries;
  const bool discovered = registry.available_report_sources(entries, error);
  if (!discovered && state.documents.empty()) {
    return false;
  }
  const std::string discoveryError = error;
  if (!discovered) {
    entries.clear();
  }
  std::set<std::string> represented;
  for (auto& entry : entries) {
    if (const auto saved = find_document(entry.request)) {
      entry.has_report = true;
      represented.insert(saved->id);
    }
  }
  for (const auto& item : state.documents) {
    if (represented.count(item.first) != 0) {
      continue;
    }
    const auto& saved = *item.second;
    ReportSourceEntry entry;
    entry.request.provider = saved.provider;
    entry.request.kind = saved.source.kind;
    entry.request.model_id = saved.source.model_id;
    entry.request.job_id = saved.source.job_id;
    entry.workspace_label = saved.source.workspace_id;
    entry.label = "Current model";
    if (saved.source.kind != "model") {
      entry.label = "Job " + std::to_string(saved.source.job_id);
    }
    entry.label += " (saved report; source unavailable)";
    entry.has_report = true;
    entry.unavailable_reason = discoveryError;
    if (entry.unavailable_reason.empty()) {
      entry.unavailable_reason = "Saved report can be viewed; its source is unavailable for generation or update.";
    }
    entries.push_back(std::move(entry));
  }
  output = std::move(entries);
  error.clear();
  return true;
}

bool ReportCore::choose_source(const ReportRequest& requested, std::string& error) {
  ReportRequest source = requested;
  {
    ReportTemplateProviderInterface* templates = nullptr;
    ReportModelProviderInterface* models = nullptr;
    std::string ignored;
    if (registry.resolve_model_reporting(source.provider, templates, models, ignored)) {
      source.provider = templates->command_prefix();
    }
  }
  std::vector<ReportSourceEntry> entries;
  if (!available_sources(entries, error)) {
    return false;
  }
  for (const auto& entry : entries) {
    if (entry.request.provider != source.provider || entry.request.kind != source.kind ||
        entry.request.job_id != source.job_id || entry.request.model_id != source.model_id) {
      continue;
    }
    const auto matches = [&](const ReportRequest& candidate) {
      return candidate.provider == source.provider && candidate.kind == source.kind &&
        candidate.model_id == source.model_id && candidate.job_id == source.job_id;
    };
    if (matches(configuration_request())) {
      error.clear();
      return true;
    }
    if (const auto saved = find_document(source)) {
      return activate_document(saved->id, error);
    }
    if (!entry.available) {
      error = entry.unavailable_reason;
      return false;
    }
    ReportTemplateProviderInterface* templates = nullptr;
    ReportModelProviderInterface* models = nullptr;
    ReportTemplate definition;
    if (!registry.resolve_model_reporting(source.provider, templates, models, error) || !templates->describe(definition, error)) {
      return false;
    }
    ReportConfigurationResolver::add_common_options(definition);
    ReportRequest pending = source;
    pending.overrides = definition.defaults;
    if (auto* store = registry.report_profiles()) {
      ReportProfile profile;
      bool found = false;
      if (!store->default_profile(definition.workspace_id, profile, found, error)) {
        return false;
      }
      if (found) {
        if (profile.template_id != definition.id || profile.template_revision != definition.revision ||
            !ReportConfigurationResolver::resolve(definition, profile.configuration, pending.overrides, error)) {
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

std::shared_ptr<const ReportDocument> ReportCore::find_document(const ReportRequest& source, const std::string& workspace) const
{
  std::string owner = workspace;
  if (owner.empty()) {
    ReportTemplateProviderInterface* templates = nullptr;
    ReportModelProviderInterface* models = nullptr;
    ReportTemplate definition;
    std::string ignored;
    if (registry.resolve_model_reporting(source.provider, templates, models, ignored) && templates->describe(definition, ignored)) {
      owner = definition.workspace_id;
    }
  }
  // Source identity excludes title, profile, input fingerprints and run identity.
  // Those may change when Generate replaces the report for the same model/job.
  for (const auto& item : state.documents) {
    const auto& saved = *item.second;
    bool sameOwner = saved.source.workspace_id == owner;
    if (owner.empty()) {
      sameOwner = saved.provider == source.provider;
    }
    bool sameSource = saved.source.model_id == source.model_id;
    if (source.kind == "job") {
      sameSource = saved.source.job_id == source.job_id;
    }
    if (sameOwner && saved.source.kind == source.kind && sameSource) {
      return item.second;
    }
  }
  return nullptr;
}

bool ReportCore::activate_document(const std::string& id, std::string& error) {
  const auto found = state.documents.find(id);
  if (found == state.documents.end()) {
    error = "Unknown report ID: " + id;
    return false;
  }
  if (state.configuration_changed && !state.active_id.empty()) {
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
  state.configuration_changed = draft != state.drafts.end() || state.units_pending_update.count(id) != 0;
  if (draft != state.drafts.end())
  {
    state.request = draft->second;
  }
  error.clear();
  return true;
}
