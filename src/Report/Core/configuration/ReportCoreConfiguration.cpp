#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportTemplateProviderInterface.hpp"
#include <utility>

void ReportCore::units_changed()
{
  for (const auto& entry : state.documents)
  {
    state.units_pending_update.insert(entry.first);
  }
  state.configuration_changed = true;
  if (state.pending_source)
  {
    SimUnits::WorkspaceUnitContext context;
    std::string error;
    if (framework_units().workspace_context("report", context, error))
    {
      state.pending_request.overrides.units = ReportUnitSettings::capture(context);
    }
  }
}

bool ReportCore::apply_units(const ReportConfiguration& configuration, std::string& error)
{
  if (!configuration.units)
  {
    return true;
  }
  SimUnits::WorkspaceUnitContext context;
  if (!framework_units().workspace_context("report", context, error))
  {
    return false;
  }
  if (ReportUnitSettings::capture(context).to_json() == configuration.units->to_json())
  {
    return true;
  }
  configuration.units->apply_to(context);
  if (!framework_units().set_workspace_context(context, error))
  {
    return false;
  }
  units_changed();
  return true;
}

bool ReportCore::describe(ReportTemplate& output, std::string& error) const
{
  ReportTemplateProviderInterface* templates = nullptr;
  ReportModelProviderInterface* models = nullptr;
  std::string provider = configuration_request().provider;
  if (provider.empty()) {
    provider = "ccx";
  }
  if (!registry.resolve_model_reporting(provider, templates, models, error)) {
    return false;
  }
  ReportTemplate candidate;
  if (!templates->describe(candidate, error)) {
    return false;
  }
  ReportConfigurationResolver::add_common_options(candidate);
  ReportConfiguration validated;
  if (!ReportConfigurationResolver::resolve(candidate, candidate.defaults, validated, error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}

bool ReportCore::configure(const std::string& key, const std::string& value, std::string& error) {
  ReportConfiguration patch;
  patch.values[key] = value;
  return configure_patch(patch, error);
}

bool ReportCore::configure_patch(const ReportConfiguration& patch, std::string& error) {
  ReportTemplate definition;
  if (!describe(definition, error)) {
    return false;
  }
  auto candidate = configuration_request().overrides;
  if (patch.replace_company_logo) {
    candidate.company_logo = patch.company_logo;
  }
  for (const auto& entry : patch.values) {
    candidate.values[entry.first] = entry.second;
  }
  ReportConfiguration resolved;
  if (!ReportConfigurationResolver::resolve(definition, candidate, resolved, error)) {
    return false;
  }
  if (state.pending_source) {
    state.pending_request.overrides = std::move(resolved);
  } else {
    state.request.overrides = std::move(resolved);
    state.configuration_changed = true;
  }
  return true;
}

bool ReportCore::validate_configuration_patch(const ReportConfiguration& patch, std::string& error) const
{
  ReportTemplate definition;
  if (!describe(definition, error)) {
    return false;
  }
  auto candidate = configuration_request().overrides;
  if (patch.replace_company_logo) {
    candidate.company_logo = patch.company_logo;
  }
  for (const auto& entry : patch.values) {
    candidate.values[entry.first] = entry.second;
  }
  ReportConfiguration resolved;
  return ReportConfigurationResolver::resolve(definition, candidate, resolved, error);
}
