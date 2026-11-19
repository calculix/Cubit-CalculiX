#include "ReportApplicationInterface.hpp"
#include "ReportApplicationPreviewState.hpp"
#include "Report/Core/ReportSession.hpp"
#include "Report/Core/ReportUnitValidation.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/GUI/Progress/ReportNativeProgress.hpp"
#include <QScopedValueRollback>
#include <utility>
#include <algorithm>

ReportApplicationInterface::PreviewState::PreviewState()
  : previewPage(ReportUserOptions::initial_preview_page()),
    previewZoom(ReportUserOptions::initial_preview_zoom()),
    previewMode(ReportUserOptions::initial_preview_mode())
{
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
    previewState->previewPage = page;
    ++previewState->previewPageRevision;
  }
  if (zoom != -1) {
    previewState->previewZoom = zoom;
    previewState->previewMode = "percent";
  }
  if (!fit.empty()) {
    previewState->previewMode = fit;
    ++previewState->previewFitRevision;
  }
  if (page == -1 && zoom == -1 && fit.empty()) {
    ++previewState->previewWindowRevision;
  }
  ++previewState->previewRevision;
  error.clear();
  return true;
}

int ReportApplicationInterface::preview_page() const noexcept
{
  return previewState->previewPage;
}

int ReportApplicationInterface::preview_zoom() const noexcept
{
  return previewState->previewZoom;
}

std::uint64_t ReportApplicationInterface::preview_revision() const noexcept
{
  return previewState->previewRevision;
}

std::shared_ptr<const ReportDocumentLayout> ReportApplicationInterface::prepared_layout() const noexcept
{
  if (session().pending_source || previewState->layout_source != document())
  {
    return {};
  }
  return previewState->cached_layout;
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
  previewState->cached_layout = std::move(layout);
  previewState->layout_source.reset();
  if (previewState->cached_layout)
  {
    previewState->layout_source = document();
    previewState->previewPage = std::clamp(previewState->previewPage, 1, std::max(1, previewState->cached_layout->page_count()));
  }
}

bool ReportApplicationInterface::prepare_selected_layout(std::string& error)
{
  if (!guiAvailable || !document() || session().pending_source)
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
  output = previewState->cached_layout;
  progress.finish();
  error.clear();
  return true;
}

const std::string& ReportApplicationInterface::preview_mode() const noexcept
{
  return previewState->previewMode;
}

std::uint64_t ReportApplicationInterface::preview_window_revision() const noexcept
{
  return previewState->previewWindowRevision;
}

std::uint64_t ReportApplicationInterface::preview_page_revision() const noexcept
{
  return previewState->previewPageRevision;
}

std::uint64_t ReportApplicationInterface::preview_fit_revision() const noexcept
{
  return previewState->previewFitRevision;
}
