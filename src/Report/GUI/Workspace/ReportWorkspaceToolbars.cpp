#include "ReportWorkspace.hpp"
#include "ReportToolbar.hpp"
#include "Report/GUI/Export/ReportExportDialog.hpp"
#include "CubitMessage.hpp"
#include <QAction>
#include <QDockWidget>
#include <QMenu>

void ReportWorkspace::create_toolbar(const QIcon& pdfIcon, const QIcon& odtIcon)
{
  toolbar = new ReportToolbar(this, pdfIcon, odtIcon, [this](const std::string& format) {
    std::string error;
    if (!ReportExportDialog::run(this, format, submit, error)) {
      PRINT_ERROR("Cannot export report: %s\n", error.c_str());
    }
  });
}

void ReportWorkspace::create_menu()
{
  menu = new QMenu(tr("Report"), this);
  connect(menu->addAction(tr("Open preview / Export")), &QAction::triggered, this, [this]() {
    dispatch("report preview");
  });
}

void ReportWorkspace::workspace_view(WorkspaceView& view) {
  view.id = "report";
  view.label = "Report";
  view.icon = icon;
  view.graphicsPage = this;
  view.useNativeNavigation = false;
  view.dockLayout = WorkspaceDockLayout::StackedLeft;
  view.docks = {treeDock};
  view.menuActions = {menu->menuAction()};
  view.toolbars = {toolbar.data()};
  view.modelManagementToolbar = toolbar.data();
  view.activationChanged = [widget = QPointer<ReportWorkspace>(this)](bool active) {
    if (active && widget) {
      widget->refresh();
    }
  };
}

bool ReportWorkspace::bind_native_toolbar(Claro* host, std::string& error) {
  return toolbar->bind_native_actions(host, error);
}
