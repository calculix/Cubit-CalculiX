#include "ReportApplicationInterface.hpp"
#include "Report/GUI/ReportNativeProgress.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include "Report/Core/ReportUnitValidation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QFileInfo>
#include "Report/Core/ReportLogo.hpp"
#include <utility>
#include "Report/Core/ReportProjectPersistence.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Document/ReportPdfExporter.hpp"
#include "Report/Document/ReportOdtExporter.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include <QFileInfo>
#include <exception>
#include <algorithm>

ReportApplicationInterface::ReportApplicationInterface(ReportProviderRegistryInterface& registry)
  : core(registry), previewPage(ReportUserOptions::initial_preview_page()), previewZoom(ReportUserOptions::initial_preview_zoom()), previewMode(ReportUserOptions::initial_preview_mode()) {
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
    core.units_changed();
  }
  return true;
}

const ReportSession& ReportApplicationInterface::session() const noexcept
{
  return core.session();
}

std::shared_ptr<const ReportDocument> ReportApplicationInterface::document() const noexcept
{
  return core.document();
}

bool ReportApplicationInterface::describe(ReportTemplate& output, std::string& error) const
{
  return core.describe(output, error);
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
  if (!core.update(error, &progress, prepare))
  {
    return false;
  }
  publish_layout(std::move(candidateLayout));
  ++previewRevision;
  progress.finish();
  return true;
}

bool ReportApplicationInterface::configure(const std::string& key, const std::string& value, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  return core.configure(key, value, error);
}

bool ReportApplicationInterface::note(const std::string& section, const std::string& value, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if (!core.note(section, value, error))
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
  return core.profile(action, name, output, error);
}

bool ReportApplicationInterface::request_preview(std::string& error, int page, int zoom, const std::string& fit) {
  if (session().pending_source) {
    error = "Generate the selected source before opening its report preview.";
    return false;
  }
  if (!guiAvailable || !document()) {
    error = "Report preview needs a generated document and the Report GUI component.";
    return false;
  }
  std::shared_ptr<const ReportDocumentLayout> documentLayout;
  if (!layout(documentLayout, error)) {
    return false;
  }
  if ((!fit.empty() && (fit != "width" && fit != "page")) || (!fit.empty() && zoom != -1) ||
      (page != -1 && (page < 1 || page > documentLayout->page_count())) ||
      (zoom != -1 && (zoom < 1 || zoom > 300))) {
    error = "Preview page must exist and zoom must be between 1 and 300 percent.";
    return false;
  }
  if (page != -1) {
    previewPage = page;
    ++previewPageRevision;
  }
  if (zoom != -1) {
    previewZoom = zoom;
    previewMode = "percent";
  }
  if (!fit.empty()) {
    previewMode = fit;
    ++previewFitRevision;
  }
  if (page == -1 && zoom == -1 && fit.empty()) {
    ++previewWindowRevision;
  }
  ++previewRevision;
  error.clear();
  return true;
}

int ReportApplicationInterface::preview_page() const noexcept
{
  return previewPage;
}

int ReportApplicationInterface::preview_zoom() const noexcept
{
  return previewZoom;
}

std::uint64_t ReportApplicationInterface::preview_revision() const noexcept
{
  return previewRevision;
}

std::shared_ptr<const ReportDocumentLayout> ReportApplicationInterface::prepared_layout() const noexcept
{
  if (core.session().pending_source || layout_source != core.document())
  {
    return {};
  }
  return cached_layout;
}

bool ReportApplicationInterface::prepare_layout(const ReportDocument& document, ReportProgress& progress,
  std::shared_ptr<ReportDocumentLayout>& output, std::string& error) const
{
  if (!progress.update(ReportProgressStage::PreviewContent, "Preparing report preview", 0, 0, error) ||
      !ReportUnitValidation::document(document, error))
  {
    return false;
  }
  auto candidate = std::make_shared<ReportDocumentLayout>();
  if (!candidate->prepare(document, error, &progress))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}

void ReportApplicationInterface::publish_layout(std::shared_ptr<ReportDocumentLayout> layout)
{
  cached_layout = std::move(layout);
  layout_source.reset();
  if (cached_layout)
  {
    layout_source = core.document();
    previewPage = std::clamp(previewPage, 1, std::max(1, cached_layout->page_count()));
  }
}

bool ReportApplicationInterface::prepare_selected_layout(std::string& error)
{
  if (!guiAvailable || !core.document() || core.session().pending_source)
  {
    return true;
  }
  std::shared_ptr<const ReportDocumentLayout> prepared;
  return layout(prepared, error);
}

bool ReportApplicationInterface::layout(std::shared_ptr<const ReportDocumentLayout>& output, std::string& error)
{
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if (const auto ready = prepared_layout())
  {
    output = ready;
    error.clear();
    return true;
  }
  const auto snapshot = document();
  if (!snapshot || session().pending_source)
  {
    error = "No generated report is selected.";
    return false;
  }
  QScopedValueRollback<bool> operation(generating, true);
  ReportNativeProgress progress(guiAvailable, ReportProgressStage::PreviewContent);
  std::shared_ptr<ReportDocumentLayout> candidate;
  if (!prepare_layout(*snapshot, progress, candidate, error))
  {
    return false;
  }
  publish_layout(std::move(candidate));
  output = cached_layout;
  progress.finish();
  error.clear();
  return true;
}

bool ReportApplicationInterface::export_document(const std::string& format, const std::string& filename, std::string& error, bool overwrite) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if ((format != "pdf" && format != "odt" && format != "both") || QString::fromStdString(filename).trimmed().isEmpty()) {
    error = "Report export requires a valid format and a non-empty destination.";
    return false;
  }
  std::vector<std::string> targets = {filename};
  if (format == "both") {
    targets = {filename + ".pdf", filename + ".odt"};
  }
  // Preflight every destination before writing either output of a paired export.
  for (const auto& target : targets) {
    const QFileInfo file(QString::fromStdString(target));
    if (file.isDir()) {
      error = "Report destination is a directory: " + target;
      return false;
    }
    if (!overwrite && (file.exists() || file.isSymLink())) {
      error = "Report destination already exists; use overwrite to replace it: " + target;
      return false;
    }
  }
  if (session().configuration_changed || session().pending_source) {
    error = "Generate the selected source or update its changed configuration before exporting.";
    return false;
  }
  std::shared_ptr<const ReportDocumentLayout> documentLayout;
  if (!layout(documentLayout, error)) {
    return false;
  }
  if (format == "pdf") {
    return ReportPdfExporter().write(*documentLayout, filename, error);
  }
  if (format == "odt") {
    return ReportOdtExporter().write(*documentLayout, filename, error);
  }
  if (format == "both") {
    if (!ReportPdfExporter().write(*documentLayout, filename + ".pdf", error)) {
      return false;
    }
    if (!ReportOdtExporter().write(*documentLayout, filename + ".odt", error)) {
      error = "PDF exported; ODT export failed: " + error;
      return false;
    }
    return true;
  }
  error = "Unknown report export format.";
  return false;
}

bool ReportApplicationInterface::prepare_project_read(const std::string& filename, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  preparedProject.reset();
  try {
    HDF5Tool file(filename, false);
    ReportSession candidate;
    if (!ReportProjectPersistence().read(file, candidate, error)) {
      return false;
    }
    if (!file.close()) {
      error = "Cannot close saved Report project data.";
      return false;
    }
    preparedProject = std::make_unique<ReportSession>(std::move(candidate));
    return true;
  }
  catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception) {
    error = exception.what();
  }
  return false;
}

bool ReportApplicationInterface::write_project(const std::string& filename, std::string& error) const
{
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  try {
    HDF5Tool file(filename, false);
    if (!ReportProjectPersistence().write(file, core.session(), error)) {
      return false;
    }
    if (!file.close()) {
      error = "Cannot close the saved report project data.";
      return false;
    }
    return true;
  }
  catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception) {
    error = exception.what();
  }
  return false;
}

void ReportApplicationInterface::reset()
{
  if (generating)
  {
    PRINT_WARNING("Cannot reset Report while a Report operation is running.\n");
    return;
  }
  core.reset();
  cached_layout.reset();
  layout_source.reset();
  previewPage = ReportUserOptions::initial_preview_page();
  previewZoom = ReportUserOptions::initial_preview_zoom();
  previewMode = ReportUserOptions::initial_preview_mode();
}

void ReportApplicationInterface::set_gui_available(bool available) noexcept
{
  guiAvailable = available;
}

const ReportRequest& ReportApplicationInterface::configuration_request() const noexcept
{
  return core.configuration_request();
}

bool ReportApplicationInterface::available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const
{
  return core.available_sources(output, error);
}

bool ReportApplicationInterface::choose_source(const ReportRequest& source, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  const auto before = core.document();
  if (!core.choose_source(source, error)) {
    return false;
  }
  if (!prepare_selected_layout(error))
  {
    return false;
  }
  if (core.document() != before) {
    previewPage = ReportUserOptions::initial_preview_page();
    ++previewPageRevision;
    ++previewRevision;
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
  if (!core.generate_request(source, profile, error, &progress, prepare))
  {
    return false;
  }
  previewPage = ReportUserOptions::initial_preview_page();
  publish_layout(std::move(candidateLayout));
  ++previewPageRevision;
  ++previewRevision;
  progress.finish();
  return true;
}

bool ReportApplicationInterface::configure_patch(const ReportConfiguration& patch, std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  return core.configure_patch(patch, error);
}

const std::string& ReportApplicationInterface::preview_mode() const noexcept
{
  return previewMode;
}

std::uint64_t ReportApplicationInterface::preview_window_revision() const noexcept
{
  return previewWindowRevision;
}

std::uint64_t ReportApplicationInterface::preview_page_revision() const noexcept
{
  return previewPageRevision;
}

bool ReportApplicationInterface::validate_configuration_patch(const ReportConfiguration& patch, std::string& error) const
{
  return core.validate_configuration_patch(patch, error);
}

bool ReportApplicationInterface::load_logo(const std::string& filename, ReportImage& output, std::string& error) const
{
  return ReportLogo::load(filename, output, error);
}

std::uint64_t ReportApplicationInterface::preview_fit_revision() const noexcept
{
  return previewFitRevision;
}

bool ReportApplicationInterface::commit_project_read(std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  error.clear();
  if (!preparedProject) {
    error = "No Report project restore was prepared.";
    return false;
  }
  QScopedValueRollback<bool> operation(generating, true);
  ReportNativeProgress progress(guiAvailable, ReportProgressStage::PreviewContent);
  std::shared_ptr<ReportDocumentLayout> candidateLayout;
  const auto active = preparedProject->documents.find(preparedProject->active_id);
  if (guiAvailable && !preparedProject->pending_source && active != preparedProject->documents.end() &&
      !prepare_layout(*active->second, progress, candidateLayout, error))
  {
    return false;
  }
  core.restore(std::move(*preparedProject));
  preparedProject.reset();
  publish_layout(std::move(candidateLayout));
  ++previewRevision;
  progress.finish();
  return true;
}
void ReportApplicationInterface::discard_project_read() noexcept
{
  preparedProject.reset();
}
bool ReportApplicationInterface::read_project(const std::string& filename, std::string& error) {
  return prepare_project_read(filename,error) && commit_project_read(error);
}

bool ReportApplicationInterface::add_image(const ReportImageRequest& request,const std::string& caption,std::string& error) {
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if (!core.add_image(request,caption,error)) {
    return false;
  }
  if (!prepare_selected_layout(error))
  {
    return false;
  }
  ++previewRevision;
  return true;
}
