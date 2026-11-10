#include "ReportApplicationInterface.hpp"
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
  : core(registry), previewPage(ReportUserOptions::initial_preview_page()), previewZoom(ReportUserOptions::initial_preview_zoom()), previewMode(ReportUserOptions::initial_preview_mode())
{
}

ReportApplicationInterface::~ReportApplicationInterface() = default;

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

bool ReportApplicationInterface::generate(const std::string& provider, int job, const std::string& profile, std::string& error)
{
  if (!core.generate(provider, job, profile, error))
  {
    return false;
  }
  previewPage = ReportUserOptions::initial_preview_page();
  ++previewPageRevision;
  ++previewRevision;
  return true;
}

bool ReportApplicationInterface::update(std::string& error)
{
  return core.update(error);
}

bool ReportApplicationInterface::configure(const std::string& key, const std::string& value, std::string& error)
{
  return core.configure(key, value, error);
}

bool ReportApplicationInterface::note(const std::string& section, const std::string& value, std::string& error)
{
  return core.note(section, value, error);
}

bool ReportApplicationInterface::profile(const std::string& action, const std::string& name, std::vector<ReportProfile>& output, std::string& error)
{
  return core.profile(action, name, output, error);
}

bool ReportApplicationInterface::request_preview(std::string& error, int page, int zoom, const std::string& fit)
{
  if (session().pending_source)
  {
    error = "Generate the selected source before opening its report preview.";
    return false;
  }
  if (!guiAvailable || !document())
  {
    error = "Report preview needs a generated document and the Report GUI component.";
    return false;
  }
  std::shared_ptr<const ReportDocumentLayout> documentLayout;
  if (!layout(documentLayout, error))
  {
    return false;
  }
  if ((!fit.empty() && (fit != "width" && fit != "page")) || (!fit.empty() && zoom != -1) ||
      (page != -1 && (page < 1 || page > documentLayout->page_count())) ||
      (zoom != -1 && (zoom < 1 || zoom > 300)))
  {
    error = "Preview page must exist and zoom must be between 1 and 300 percent.";
    return false;
  }
  if (page != -1)
  {
    previewPage = page;
    ++previewPageRevision;
  }
  if (zoom != -1)
  {
    previewZoom = zoom;
    previewMode = "percent";
  }
  if (!fit.empty())
  {
    previewMode = fit;
    ++previewFitRevision;
  }
  if (page == -1 && zoom == -1 && fit.empty())
  {
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

bool ReportApplicationInterface::layout(std::shared_ptr<const ReportDocumentLayout>& output, std::string& error)
{
  const auto snapshot = document();
  if (!snapshot)
  {
    error = "No generated report is selected.";
    return false;
  }
  if (snapshot != layout_source || !cached_layout)
  {
    auto candidate = std::make_shared<ReportDocumentLayout>();
    if (!candidate->prepare(*snapshot, error))
    {
      return false;
    }
    cached_layout = std::move(candidate);
    layout_source = snapshot;
    previewPage = std::clamp(previewPage, 1, std::max(1, cached_layout->page_count()));
  }
  output = cached_layout;
  error.clear();
  return true;
}

bool ReportApplicationInterface::export_document(const std::string& format, const std::string& filename, std::string& error, bool overwrite)
{
  if ((format != "pdf" && format != "odt" && format != "both") || QString::fromStdString(filename).trimmed().isEmpty())
  {
    error = "Report export requires a valid format and a non-empty destination.";
    return false;
  }
  const std::vector<std::string> targets = format == "both" ?
    std::vector<std::string>{filename + ".pdf", filename + ".odt"} : std::vector<std::string>{filename};
  // Preflight every destination before writing either output of a paired export.
  for (const auto& target : targets)
  {
    const QFileInfo file(QString::fromStdString(target));
    if (file.isDir())
    {
      error = "Report destination is a directory: " + target;
      return false;
    }
    if (!overwrite && (file.exists() || file.isSymLink()))
    {
      error = "Report destination already exists; use overwrite to replace it: " + target;
      return false;
    }
  }
  if (session().configuration_changed || session().pending_source)
  {
    error = "Generate the selected source or update its changed configuration before exporting.";
    return false;
  }
  std::shared_ptr<const ReportDocumentLayout> documentLayout;
  if (!layout(documentLayout, error))
  {
    return false;
  }
  if (format == "pdf")
  {
    return ReportPdfExporter().write(*documentLayout, filename, error);
  }
  if (format == "odt")
  {
    return ReportOdtExporter().write(*documentLayout, filename, error);
  }
  if (format == "both")
  {
    if (!ReportPdfExporter().write(*documentLayout, filename + ".pdf", error))
    {
      return false;
    }
    if (!ReportOdtExporter().write(*documentLayout, filename + ".odt", error))
    {
      error = "PDF exported; ODT export failed: " + error;
      return false;
    }
    return true;
  }
  error = "Unknown report export format.";
  return false;
}

bool ReportApplicationInterface::prepare_project_read(const std::string& filename, std::string& error)
{
  preparedProject.reset();
  try
  {
    HDF5Tool file(filename, false);
    ReportSession candidate;
    if (!ReportProjectPersistence().read(file, candidate, error))
    {
      return false;
    }
    if (!file.close())
    {
      error = "Cannot close saved Report project data.";
      return false;
    }
    preparedProject = std::make_unique<ReportSession>(std::move(candidate));
    return true;
  }
  catch (const H5::Exception& exception)
  {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  return false;
}

bool ReportApplicationInterface::write_project(const std::string& filename, std::string& error) const
{
  try
  {
    HDF5Tool file(filename, false);
    if (!ReportProjectPersistence().write(file, core.session(), error))
    {
      return false;
    }
    if (!file.close())
    {
      error = "Cannot close the saved report project data.";
      return false;
    }
    return true;
  }
  catch (const H5::Exception& exception)
  {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  return false;
}

void ReportApplicationInterface::reset()
{
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

bool ReportApplicationInterface::choose_source(const ReportRequest& source, std::string& error)
{
  const auto before = core.document();
  if (!core.choose_source(source, error))
  {
    return false;
  }
  if (core.document() != before)
  {
    previewPage = ReportUserOptions::initial_preview_page();
    ++previewPageRevision;
    ++previewRevision;
  }
  return true;
}

bool ReportApplicationInterface::generate_request(const ReportRequest& source, const std::string& profile, std::string& error)
{
  if (!core.generate_request(source, profile, error))
  {
    return false;
  }
  previewPage = ReportUserOptions::initial_preview_page();
  ++previewPageRevision;
  ++previewRevision;
  return true;
}

bool ReportApplicationInterface::configure_patch(const ReportConfiguration& patch, std::string& error)
{
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

bool ReportApplicationInterface::commit_project_read(std::string& error)
{
  error.clear();
  if (!preparedProject)
  {
    error = "No Report project restore was prepared.";
    return false;
  }
  core.restore(std::move(*preparedProject));
  preparedProject.reset();
  cached_layout.reset();
  layout_source.reset();
  return true;
}
void ReportApplicationInterface::discard_project_read() noexcept
{
  preparedProject.reset();
}
bool ReportApplicationInterface::read_project(const std::string& filename, std::string& error)
{
  return prepare_project_read(filename,error) && commit_project_read(error);
}
