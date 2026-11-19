#pragma once
#include "ReportApplicationInterface.hpp"

struct ReportApplicationInterface::PreviewState
{
  PreviewState();

  std::shared_ptr<const ReportDocument> layout_source;
  std::shared_ptr<ReportDocumentLayout> cached_layout;
  std::uint64_t previewRevision = 0;
  int previewPage;
  int previewZoom;
  std::string previewMode;
  std::uint64_t previewWindowRevision = 0;
  std::uint64_t previewPageRevision = 0;
  std::uint64_t previewFitRevision = 0;
};
