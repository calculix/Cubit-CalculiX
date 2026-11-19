#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include <utility>

bool ReportCore::profile(const std::string& action, const std::string& name, std::vector<ReportProfile>& output, std::string& error) {
  auto* store = registry.report_profiles();
  ReportTemplate definition;
  if (store == nullptr || !describe(definition, error)) {
    if (error.empty()) {
      error = "Framework report-profile storage is unavailable.";
    }
    return false;
  }
  if (action == "list") {
    return store->list(definition.workspace_id, output, error);
  }
  if (action == "reset") {
    ReportProfile profile;
    bool found = false;
    if (!store->default_profile(definition.workspace_id, profile, found, error)) {
      return false;
    }
    ReportConfiguration values = definition.defaults;
    if (found && (profile.template_id != definition.id || profile.template_revision != definition.revision ||
        !ReportConfigurationResolver::resolve(definition, profile.configuration, values, error))) {
      if (error.empty()) {
        error = "Incompatible workspace default profile.";
      }
      return false;
    }
    ReportRequest* selectedRequest = &state.request;
    if (state.pending_source) {
      selectedRequest = &state.pending_request;
    }
    auto& target = *selectedRequest;
    if (!apply_units(values, error))
    {
      return false;
    }
    target.overrides = std::move(values);
    target.profile_id.clear();
    target.profile_revision = 0;
    if (found) {
      target.profile_id = profile.id;
      target.profile_revision = profile.revision;
    }
    if (!state.pending_source) {
      state.configuration_changed = true;
    }
    return true;
  }
  if (action == "default") {
    ReportProfile profile;
    ReportConfiguration validated;
    if (!store->load(definition.workspace_id, name, profile, error) ||
        profile.template_id != definition.id || profile.template_revision != definition.revision ||
        !ReportConfigurationResolver::resolve(definition, profile.configuration, validated, error)) {
      if (error.empty()) {
        error = "Incompatible report profile.";
      }
      return false;
    }
    return store->set_default_profile(definition.workspace_id, name, error);
  }
  if (action == "save") {
    ReportProfile profile;
    profile.name = name;
    profile.workspace_id = definition.workspace_id;
    profile.template_id = definition.id;
    profile.template_revision = definition.revision;
    if (!ReportConfigurationResolver::resolve(definition, configuration_request().overrides, profile.configuration, error)) {
      return false;
    }
    SimUnits::WorkspaceUnitContext context;
    if (!framework_units().workspace_context("report", context, error))
    {
      return false;
    }
    profile.configuration.units = ReportUnitSettings::capture(context);
    if (!store->save(profile, error)) {
      return false;
    }
    output = {profile};
    return true;
  }
  if (action == "load") {
    ReportProfile profile;
    ReportConfiguration resolved;
    if (!store->load(definition.workspace_id, name, profile, error) ||
        profile.template_id != definition.id || profile.template_revision != definition.revision ||
        !ReportConfigurationResolver::resolve(definition, profile.configuration, resolved, error)) {
      if (error.empty()) {
        error = "Incompatible report profile.";
      }
      return false;
    }
    ReportRequest* selectedRequest = &state.request;
    if (state.pending_source) {
      selectedRequest = &state.pending_request;
    }
    auto& target = *selectedRequest;
    if (!apply_units(resolved, error))
    {
      return false;
    }
    target.overrides = std::move(resolved);
    target.profile_id = profile.id;
    target.profile_revision = profile.revision;
    if (!state.pending_source) {
      state.configuration_changed = true;
    }
    return true;
  }
  error = "Unknown report profile operation.";
  return false;
}
