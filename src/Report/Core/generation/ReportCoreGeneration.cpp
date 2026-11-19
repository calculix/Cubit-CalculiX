#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportTemplateProviderInterface.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include "Report/Core/ReportUnitValidation.hpp"
#include "Framework/Reporting/ReportMeshQualityContent.hpp"
#include "Framework/Reporting/ReportModelProviderInterface.hpp"
#include "Framework/Reporting/ReportResultProviderInterface.hpp"
#include "Framework/Reporting/ReportResultRequest.hpp"
#include "Framework/Reporting/ReportImageProviderInterface.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include <QDateTime>
#include <QUuid>
#include <utility>

bool ReportCore::generate(const std::string& provider, int job, const std::string& name, std::string& error, ReportProgress* progress, const ReportDocumentPreparation& prepare) {
  ReportRequest request;
  request.provider = provider;
  request.job_id = job;
  return generate_request(request, name, error, progress, prepare);
}

bool ReportCore::generate_request(const ReportRequest& source, const std::string& name, std::string& error, ReportProgress* progress, const ReportDocumentPreparation& prepare) {
  if (!ReportProgress::advance(progress, ReportProgressStage::Source, "Preparing report request", 0, 1, error))
  {
    return false;
  }
  ReportRequest request = source;
  ReportTemplateProviderInterface* templates = nullptr;
  ReportModelProviderInterface* models = nullptr;
  ReportTemplate definition;
  if ((request.kind != "job" && request.kind != "model") ||
      (request.kind == "job" && request.job_id <= 0) ||
      (request.kind == "model" && request.model_id.empty()) ||
      !registry.resolve_model_reporting(source.provider, templates, models, error) || !templates->describe(definition, error)) {
    if (error.empty()) {
      error = "Report generation requires a valid model or positive job ID.";
    }
    return false;
  }
  const auto& current = configuration_request();
  if (name.empty() && current.provider == source.provider && current.kind == source.kind &&
      current.model_id == source.model_id && current.job_id == source.job_id && !current.overrides.values.empty()) {
    request.overrides = current.overrides;
    request.profile_id = current.profile_id;
    request.profile_revision = current.profile_revision;
    return build(request, nullptr, false, error, progress, prepare);
  }
  if (name.empty()) {
    if (const auto saved = find_document(source, definition.workspace_id)) {
      const auto draft = state.drafts.find(saved->id);
      if (draft != state.drafts.end()) {
        request = draft->second;
      } else {
        request.overrides = saved->configuration;
        request.profile_id = saved->profile_id;
        request.profile_revision = saved->profile_revision;
      }
      request.provider = source.provider;
      return build(request, nullptr, false, error, progress, prepare);
    }
  }
  ReportProfile selected;
  bool found = false;
  if (auto* store = registry.report_profiles()) {
    if (!name.empty()) {
      if (!store->load(definition.workspace_id, name, selected, error)) {
        return false;
      }
      found = true;
    } else if (!store->default_profile(definition.workspace_id, selected, found, error)) {
      return false;
    }
  } else if (!name.empty()) {
    error = "Framework report profiles are unavailable.";
    return false;
  }
  if (found) {
    request.overrides = selected.configuration;
    request.profile_id = selected.id;
    request.profile_revision = selected.revision;
  }
  const ReportProfile* profile = nullptr;
  if (found) {
    profile = &selected;
  }
  return build(request, profile, false, error, progress, prepare);
}

bool ReportCore::build(const ReportRequest& request, const ReportProfile* profile, bool updating, std::string& error, ReportProgress* progress, const ReportDocumentPreparation& prepare) {
  error.clear();
  if (!ReportProgress::advance(progress, ReportProgressStage::Source, "Resolving report source and settings", 0, 1, error))
  {
    return false;
  }

  ReportTemplateProviderInterface* templates = nullptr;
  ReportModelProviderInterface* models = nullptr;
  ReportTemplate definition;
  if (!registry.resolve_model_reporting(request.provider, templates, models, error) || !templates->describe(definition, error)) {
    return false;
  }
  std::vector<ReportSourceEntry> liveSources;
  if (!models->available_sources(liveSources, error)) {
    return false;
  }
  bool sourceAvailable = false;
  for (const auto& entry : liveSources) {
    bool sameSource = entry.request.model_id == request.model_id;
    if (request.kind == "job") {
      sameSource = entry.request.job_id == request.job_id;
    }
    if (entry.request.kind == request.kind && sameSource) {
      sourceAvailable = entry.available;
      if (!sourceAvailable) {
        error = entry.unavailable_reason;
      }
      break;
    }
  }
  if (!sourceAvailable) {
    if (error.empty()) {
      error = "The selected report source is unavailable for generation or update.";
    }
    return false;
  }
  const auto previous = find_document(request, definition.workspace_id);
  if (!previous && state.documents.size() >= 1000) {
    error = "The project already contains the maximum supported number of report sources (1000).";
    return false;
  }
  ReportConfigurationResolver::add_common_options(definition);
  if (profile != nullptr && (profile->template_id != definition.id || profile->template_revision != definition.revision)) {
    error = "The saved profile requires a different report template revision.";
    return false;
  }
  ReportRequest resolved = request;
  resolved.progress = progress;
  resolved.provider = templates->command_prefix();
  if (!ReportConfigurationResolver::resolve(definition, request.overrides, resolved.overrides, error)) {
    return false;
  }
  // Explicit/default profiles choose presentation units before providers collect.
  // Unit selection remains active if generation fails; the previous document is immutable.
  SimUnits::WorkspaceUnitContext context;
  if (!framework_units().workspace_context("report", context, error))
  {
    return false;
  }
  if (profile != nullptr || (state.pending_source && !previous))
  {
    if (!apply_units(resolved.overrides, error) || !framework_units().workspace_context("report", context, error))
    {
      return false;
    }
  }
  resolved.overrides.units = ReportUnitSettings::capture(context);
  ReportSource source;
  if (!models->resolve_source(resolved, source, error)) {
    return false;
  }
  auto candidate = std::make_shared<ReportDocument>();
  if (previous != nullptr) {
    candidate->id = previous->id;
  } else {
    candidate->id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
  }
  candidate->title = resolved.overrides.values.at("title");
  candidate->provider = resolved.provider;
  candidate->template_id = definition.id;
  candidate->template_revision = definition.revision;
  candidate->profile_id = request.profile_id;
  candidate->profile_revision = request.profile_revision;
  candidate->configuration = resolved.overrides;
  candidate->generated_at = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
  if (!ReportProgress::advance(progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", 0, 0, error))
  {
    return false;
  }
  if (!models->collect(resolved, source, *candidate, error)) {
    return false;
  }
  if (candidate->source.workspace_id != definition.workspace_id || candidate->source.kind != request.kind || candidate->source.job_id != request.job_id ||
      (request.kind == "model" && candidate->source.model_id != request.model_id)) {
    error = "The provider returned report content for a different source.";
    return false;
  }
  if (previous) {
    candidate->id = previous->id;
    candidate->notes = previous->notes;
    // User figures are immutable snapshots, independent of generated content.
    for (const auto& section : previous->sections) {
      if (section.id == "user.figures") {
        candidate->sections.push_back(section);
      }
    }
  }
  if (updating && previous != nullptr) {
    if (candidate->source.workspace_id != previous->source.workspace_id ||
        candidate->source.kind != previous->source.kind || candidate->source.model_id != previous->source.model_id ||
        candidate->source.job_id != previous->source.job_id ||
        candidate->source.input_files != previous->source.input_files ||
        (request.kind == "job" && candidate->source.fingerprints != previous->source.fingerprints) ||
        candidate->source.run_identity != previous->source.run_identity ||
        candidate->source.solver_launch_id != previous->source.solver_launch_id ||
        candidate->source.setup_snapshot_id != previous->source.setup_snapshot_id ||
        (request.kind == "job" && candidate->source.snapshot_fingerprint != previous->source.snapshot_fingerprint)) {
      error = "Report sources changed. Generate again explicitly to replace the report using the current job files.";
      return false;
    }
    candidate->profile_revision = request.profile_revision;
  }
  MeshQualitySettings qualitySettings;
  std::shared_ptr<const MeshQualitySnapshot> qualitySnapshot;
  if (!MeshQualitySettings::decode(resolved.overrides, qualitySettings, error))
  {
    return false;
  }
  const bool qualityContent = qualitySettings.statistics || qualitySettings.images ||
    qualitySettings.failed_images || qualitySettings.element_values;
  if (resolved.overrides.values.count("mesh.quality.enabled") != 0 && qualitySettings.enabled &&
      qualityContent && !qualitySettings.metrics.empty())
  {
    if (!models->mesh_quality(resolved, candidate->source, qualitySnapshot, error))
    {
      return false;
    }
    if (qualitySnapshot)
    {
      if (!ReportMeshQualityContent::append(*qualitySnapshot, qualitySettings, progress, *candidate, error))
      {
        return false;
      }
    }
    else
    {
      candidate->diagnostics.push_back({"mesh.quality", "unavailable", "Mesh quality was not captured for this source."});
    }
  }
  const auto resultSelection = resolved.overrides.values.find("post.results");
  if (request.kind == "job" && resultSelection != resolved.overrides.values.end() && resultSelection->second == "true") {
    if (auto* results = registry.report_results()) {
      ReportResultRequest resultRequest;
      resultRequest.progress = progress;
      resultRequest.provider = resolved.provider;
      resultRequest.source = candidate->source;
      resultRequest.settings = resolved.overrides;
      if (!ReportProgress::advance(progress, ReportProgressStage::Results, "Collecting result tables and extrema", 0, 1, error))
      {
        return false;
      }
      if (!results->collect(resultRequest, *candidate, error)) {
        return false;
      }
    } else {
      ReportDiagnostic issue;
      issue.section_id = "post.results";
      issue.severity = "unavailable";
      issue.message = "Block summaries and optional detailed results require the Postprocessor reporting provider.";
      candidate->diagnostics.push_back(std::move(issue));
    }
  }
  const auto imageSelection = resolved.overrides.values.find("post.images");
  if (auto* images = registry.report_images(resolved.provider)) {
    if (!ReportProgress::advance(progress, ReportProgressStage::Images, "Preparing model and setup images", 0, 0, error))
    {
      return false;
    }
    ReportImageRequest imageRequest;
    imageRequest.progress = progress;
    imageRequest.provider = resolved.provider;
    imageRequest.source = candidate->source;
    imageRequest.settings = resolved.overrides;
    if (!images->collect(imageRequest, *candidate, error)) {
      return false;
    }
  }
  if (qualitySnapshot && !qualitySnapshot->blocks.empty() && (qualitySettings.images || qualitySettings.failed_images))
  {
    if (auto* images = registry.report_images("postprocessor"))
    {
      ReportImageRequest imageRequest;
      imageRequest.progress = progress;
      imageRequest.provider = "postprocessor";
      imageRequest.purpose = "mesh_quality";
      imageRequest.source = candidate->source;
      imageRequest.settings = resolved.overrides;
      imageRequest.mesh_quality = qualitySnapshot;
      if (!images->collect(imageRequest, *candidate, error))
      {
        return false;
      }
    }
    else
    {
      candidate->diagnostics.push_back({"mesh.quality", "unavailable", "Quality images require the Postprocessor image provider."});
    }
  }
  if (request.kind == "job" && imageSelection != resolved.overrides.values.end() && imageSelection->second == "true")
  {
    if (auto* images = registry.report_images("postprocessor"))
    {
      ReportImageRequest imageRequest;
      imageRequest.progress = progress;
      imageRequest.provider = "postprocessor";
      imageRequest.source = candidate->source;
      imageRequest.settings = resolved.overrides;
      if (!images->collect(imageRequest, *candidate, error))
      {
        return false;
      }
    }
    else
    {
      ReportDiagnostic issue;
      issue.section_id = "post.images";
      issue.severity = "unavailable";
      issue.message = "Result images require the Postprocessor reporting image provider.";
      candidate->diagnostics.push_back(std::move(issue));
    }
  }
  qualitySnapshot.reset();
  if (!ReportProgress::advance(progress, ReportProgressStage::Validation, "Validating generated report", 0, 1, error))
  {
    return false;
  }
  if (!ReportUnitValidation::document(*candidate, error)) {
    return false;
  }
  if (!ReportProgress::advance(progress, ReportProgressStage::Validation, "Report content complete", 1, 1, error))
  {
    return false;
  }
  if (prepare && !prepare(*candidate, error))
  {
    return false;
  }
  if (!ReportProgress::advance(progress, ReportProgressStage::Contents, "Publishing report", 0, 1, error))
  {
    return false;
  }
  // The request published in session state must never retain a borrowed sink.
  resolved.progress = nullptr;
  if (state.configuration_changed && !state.active_id.empty()) {
    state.drafts[state.active_id] = state.request;
  }
  state.drafts.erase(candidate->id);
  state.units_pending_update.erase(candidate->id);
  state.documents[candidate->id] = candidate;
  state.active_id = candidate->id;
  state.request = std::move(resolved);
  state.configuration_changed = false;
  state.pending_source = false;
  return true;
}

bool ReportCore::update(std::string& error, ReportProgress* progress, const ReportDocumentPreparation& prepare) {
  if (!document() || state.pending_source) {
    error = "Choose a source with a generated report before updating.";
    return false;
  }
  return build(state.request, nullptr, true, error, progress, prepare);
}
