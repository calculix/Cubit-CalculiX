#include <QToolBar>
#include "MenuManager.hpp"
#include "CalculiXConfigFile.hpp"
#include "Claro.hpp"
#include "ClaroFileMenu.hpp"
#include "OptionsPanel.hpp"
#include "CubitMessage.hpp"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QStyle>
#include <QUrl>
#include <exception>
#include <stdexcept>

MenuManager::MenuManager(QObject* parent) : QObject(parent)
{
}

MenuManager::~MenuManager()
{
  remove_menu_items();
}

void MenuManager::setup_workspace_menus(Claro* host, QDockWidget* modelDock, QDockWidget* propertiesDock, QToolBar* drawingToolbar, QToolBar* topologyToolbar)
{
  if (!menus.isEmpty())
  {
    return;
  }

  if (host == nullptr || host->file_menu() == nullptr || modelDock == nullptr || propertiesDock == nullptr)
  {
    throw std::runtime_error("CalculiX menus require the Cubit file handler and workspace docks.");
  }

  auto createMenu = [this, host](const QString& name, const QString& label)
  {
    auto* menu = new QMenu(label, host->menuBar());
    menu->setObjectName(name);
    menu->menuAction()->setVisible(false);
    host->menuBar()->addAction(menu->menuAction());
    menus.append(menu);
    return menu;
  };

  auto* file = createMenu("CalculiX.Menu.File", tr("&File"));
  auto* handler = host->file_menu();
  auto addFileAction = [file, host, handler](const QString& text, QStyle::StandardPixmap icon, void (ClaroFileMenu::*slot)())
  {
    auto* action = file->addAction(host->style()->standardIcon(icon), text);
    QObject::connect(action, &QAction::triggered, handler, slot);
  };

  // Keep Cubit toolbar shortcuts; these menu actions add no duplicate shortcuts.
  addFileAction(tr("&New"), QStyle::SP_FileIcon, &ClaroFileMenu::file_new);
  addFileAction(tr("&Open..."), QStyle::SP_DirOpenIcon, &ClaroFileMenu::file_open);
  addFileAction(tr("&Save"), QStyle::SP_DialogSaveButton, &ClaroFileMenu::file_save);
  addFileAction(tr("Save &As..."), QStyle::SP_DialogSaveButton, &ClaroFileMenu::file_save_as);

  auto* view = createMenu("CalculiX.Menu.View", tr("&View"));
  view->addAction(modelDock->toggleViewAction());
  view->addAction(propertiesDock->toggleViewAction());
  if (drawingToolbar != nullptr)
  {
    view->addSeparator();
    view->addAction(drawingToolbar->toggleViewAction());
  }

  if (topologyToolbar != nullptr)
  {
    view->addAction(topologyToolbar->toggleViewAction());
  }

  auto* application = createMenu("CalculiX.Menu.Application", tr("&CalculiX"));
  auto* options = application->addAction(host->style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("&Options..."));
  connect(options, &QAction::triggered, this, [guard = QPointer<Claro>(host)]
  {
    if (guard != nullptr && guard->options() != nullptr)
    {
      guard->options()->refresh_settings();
      guard->options()->show();
      guard->options()->raise();
      guard->options()->activateWindow();
    }
  });

  auto* help = createMenu("CalculiX.Menu.Help", tr("&Help"));
  auto* documentation = help->addAction(host->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("Cubit-CalculiX documentation"));
  connect(documentation, &QAction::triggered, this, &MenuManager::open_documentation);
}

QList<QAction*> MenuManager::workspace_menu_actions() const
{
  QList<QAction*> actions;

  for (const auto& menu : menus)
  {
    if (menu != nullptr)
    {
      actions.append(menu->menuAction());
    }
  }

  return actions;
}

void MenuManager::remove_menu_items()
{
  for (const auto& menu : menus)
  {
    delete menu.data();
  }

  menus.clear();
}

void MenuManager::open_documentation()
{
  QString component_directory;

  try
  {
    component_directory = QString::fromStdString(CalculiXConfigFile().componentpath);
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Unable to locate Cubit-CalculiX documentation: %s\n", error.what());
    return;
  }

  const QString index_path = QDir(component_directory).filePath("documentation/index.html");
  const QFileInfo index(index_path);
  if (!index.isFile() || !index.isReadable())
  {
    PRINT_ERROR("Cubit-CalculiX documentation is missing or unreadable: %s\n", index_path.toUtf8().constData());
    return;
  }

  // Success means the desktop accepted the request, not that rendering completed.
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(index.absoluteFilePath())))
  {
    PRINT_ERROR("Unable to open Cubit-CalculiX documentation: %s\n", index_path.toUtf8().constData());
    return;
  }
}
