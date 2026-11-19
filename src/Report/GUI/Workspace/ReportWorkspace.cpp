#include "ReportWorkspace.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Core/ReportSession.hpp"
#include "ReportToolbar.hpp"
#include <QDockWidget>
#include <QVBoxLayout>
#include <QScopedValueRollback>
#include <utility>

ReportWorkspace::ReportWorkspace(QWidget* host, std::weak_ptr<ReportApplicationInterface> active,
  std::function<bool(const std::string&, std::string&)> send,
  std::function<bool(const std::string&, std::string&)> quiet, const QIcon& image,
  const QIcon& pdfIcon, const QIcon& odtIcon)
  : QWidget(host), application(std::move(active)), submit(std::move(send)), silent(std::move(quiet)), icon(image)
{
  setObjectName("ReportWorkspace");
  create_toolbar(pdfIcon, odtIcon);
  auto* outer = new QVBoxLayout(this);
  create_source_controls(outer);
  create_preview_controls(outer);
  create_content_and_preview(host);
  create_menu();
  refresh();
}

ReportWorkspace::~ReportWorkspace() {
  delete toolbar.data();
  delete treeDock.data();
}

void ReportWorkspace::refresh()
{
  if (refreshing)
  {
    return;
  }
  const auto active = application.lock();
  // Transient busy refreshes must not clear the last committed preview.
  if (!active || active->busy())
  {
    return;
  }
  QScopedValueRollback<bool> operation(refreshing, true);
  const auto& state = active->session();
  toolbar->set_export_enabled(active->document() != nullptr && !state.configuration_changed && !state.pending_source);
  std::string error;
  refresh_sources(*active, error);
  refresh_preview(*active, error);
}
