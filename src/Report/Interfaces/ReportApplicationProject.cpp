#include "ReportApplicationInterface.hpp"
#include "ReportApplicationPreviewState.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Report/GUI/Progress/ReportNativeProgress.hpp"
#include "Report/Core/ReportProjectPersistence.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include <utility>
#include <exception>

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
    if (!ReportProjectPersistence().write(file, core->session(), error)) {
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
  core->restore(std::move(*preparedProject));
  preparedProject.reset();
  publish_layout(std::move(candidateLayout));
  ++previewState->previewRevision;
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

void ReportApplicationInterface::reset()
{
  if (generating)
  {
    PRINT_WARNING("Cannot reset Report while a Report operation is running.\n");
    return;
  }
  core->reset();
  previewState->cached_layout.reset();
  previewState->layout_source.reset();
  previewState->previewPage = ReportUserOptions::initial_preview_page();
  previewState->previewZoom = ReportUserOptions::initial_preview_zoom();
  previewState->previewMode = ReportUserOptions::initial_preview_mode();
}
