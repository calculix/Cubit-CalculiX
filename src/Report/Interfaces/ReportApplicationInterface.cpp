#include "ReportApplicationInterface.hpp"
#include "ReportApplicationPreviewState.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Report/Core/ReportLogo.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/GUI/Progress/ReportNativeProgress.hpp"
#include <QScopedValueRollback>
#include <utility>
#include "CubitMessage.hpp"

ReportApplicationInterface::ReportApplicationInterface(ReportProviderRegistryInterface& registry)
  : core(std::make_unique<ReportCore>(registry)), previewState(std::make_unique<PreviewState>())
{
}

ReportApplicationInterface::~ReportApplicationInterface() = default;

bool ReportApplicationInterface::busy() const noexcept
{
  return generating;
}

bool ReportApplicationInterface::configure_units(const std::string& system, const std::string& quantity, const std::string& unit, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  SimUnits::WorkspaceUnitContext before;
  if (!framework_units().workspace_context("report", before, error))
  {
    return false;
  }
  bool changed = false;
  if (!system.empty()) {
    changed = framework_units().set_workspace_system("report", system, error);
  } else {
    changed = framework_units().set_display_unit("report", quantity, unit, error);
  }
  if (!changed) {
    return false;
  }
  SimUnits::WorkspaceUnitContext after;
  if (!framework_units().workspace_context("report", after, error))
  {
    return false;
  }
  if (ReportUnitSettings::capture(before).to_json() != ReportUnitSettings::capture(after).to_json())
  {
    core->units_changed();
  }
  return true;
}

const ReportSession& ReportApplicationInterface::session() const noexcept
{
  return core->session();
}

std::shared_ptr<const ReportDocument> ReportApplicationInterface::document() const noexcept
{
  return core->document();
}

bool ReportApplicationInterface::describe(ReportTemplate& output, std::string& error) const
{
  return core->describe(output, error);
}

bool ReportApplicationInterface::generate(const std::string& provider, int job, const std::string& profile, std::string& error) {
  ReportRequest request;
  request.provider = provider;
  request.job_id = job;
  return generate_request(request, profile, error);
}

bool ReportApplicationInterface::update(std::string& error) {
  if (generating)
  {
    error = "A Report operation is already running.";
    return false;
  }
  QScopedValueRollback<bool> operation(generating, true);
  ReportNativeProgress progress(guiAvailable);
  std::shared_ptr<ReportDocumentLayout> candidateLayout;
  const auto prepare = [&](const ReportDocument& document, std::string& error)
  {
    if (!guiAvailable)
    {
      return true;
    }
    return prepare_layout(document, progress, candidateLayout, error);
  };
  if (!core->update(error, &progress, prepare))
  {
    return false;
  }
  publish_layout(std::move(candidateLayout));
  ++previewState->previewRevision;
  progress.finish();
  return true;
}

bool ReportApplicationInterface::configure(const std::string& key, const std::string& value, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  return core->configure(key, value, error);
}

bool ReportApplicationInterface::note(const std::string& section, const std::string& value, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if (!core->note(section, value, error))
  {
    return false;
  }
  return prepare_selected_layout(error);
}

bool ReportApplicationInterface::profile(const std::string& action, const std::string& name, std::vector<ReportProfile>& output, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  return core->profile(action, name, output, error);
}

void ReportApplicationInterface::set_gui_available(bool available) noexcept
{
  guiAvailable = available;
}

const ReportRequest& ReportApplicationInterface::configuration_request() const noexcept
{
  return core->configuration_request();
}

bool ReportApplicationInterface::available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  return core->available_sources(output, error);
}

bool ReportApplicationInterface::choose_source(const ReportRequest& source, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  const auto before = core->document();
  if (!core->choose_source(source, error)) {
    return false;
  }
  if (!prepare_selected_layout(error))
  {
    return false;
  }
  if (core->document() != before) {
    previewState->previewPage = ReportUserOptions::initial_preview_page();
    ++previewState->previewPageRevision;
    ++previewState->previewRevision;
  }
  return true;
}

bool ReportApplicationInterface::generate_request(const ReportRequest& source, const std::string& profile, std::string& error) {
  if (generating)
  {
    error = "A Report operation is already running.";
    return false;
  }
  QScopedValueRollback<bool> operation(generating, true);
  ReportNativeProgress progress(guiAvailable);
  std::shared_ptr<ReportDocumentLayout> candidateLayout;
  const auto prepare = [&](const ReportDocument& document, std::string& error)
  {
    if (!guiAvailable)
    {
      return true;
    }
    return prepare_layout(document, progress, candidateLayout, error);
  };
  if (!core->generate_request(source, profile, error, &progress, prepare))
  {
    return false;
  }
  previewState->previewPage = ReportUserOptions::initial_preview_page();
  publish_layout(std::move(candidateLayout));
  ++previewState->previewPageRevision;
  ++previewState->previewRevision;
  progress.finish();
  return true;
}

bool ReportApplicationInterface::configure_patch(const ReportConfiguration& patch, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  return core->configure_patch(patch, error);
}

bool ReportApplicationInterface::validate_configuration_patch(const ReportConfiguration& patch, std::string& error) const
{
  return core->validate_configuration_patch(patch, error);
}

bool ReportApplicationInterface::load_logo(const std::string& filename, ReportImage& output, std::string& error) const
{
  return ReportLogo::load(filename, output, error);
}

bool ReportApplicationInterface::add_image(const ReportImageRequest& request,const std::string& caption,std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if (!core->add_image(request,caption,error)) {
    return false;
  }
  if (!prepare_selected_layout(error))
  {
    return false;
  }
  ++previewState->previewRevision;
  return true;
}
