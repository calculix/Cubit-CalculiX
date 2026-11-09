#include "CubitWorkspaceHost.hpp"
#include "Framework/Core/CubitSimVersion.hpp"
#include "DocumentationManager.hpp"
#include "WorkspaceGraphicsStack.hpp"
#include "Claro.hpp"
#include "CommandWindow.hpp"
#include "NavigationModel.hpp"
#include "TaskPanel.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QGroupBox>
#include <QMenu>
#include <QMenuBar>
#include <QScopedValueRollback>
#include <QTabWidget>
#include <QThread>
#include <QToolBar>
#include <QSettings>

namespace
{
QDockWidget* enclosingDock(QWidget* widget, QWidget* host)
{
  for (auto* parent = widget; parent != nullptr && parent != host; parent = parent->parentWidget())
  {
    if (auto* dock = qobject_cast<QDockWidget*>(parent))
    {
      return dock;
    }
  }

  return nullptr;
}
}

CubitWorkspaceHost::CubitWorkspaceHost() : graphics(std::make_unique<WorkspaceGraphicsStack>())
{
}

CubitWorkspaceHost::~CubitWorkspaceHost()
{
  graphics->detach();
  if (can_access_ui() && !titleSuffix.isEmpty())
  {
    QString title = gui->get_title();
    const auto position = title.lastIndexOf(titleSuffix);
    if (position >= 0)
    {
      title.remove(position, titleSuffix.size());
      gui->set_title(title);
    }
  }
}

bool CubitWorkspaceHost::attach(Claro* host, std::string& error)
{
  if (host == nullptr || QThread::currentThread() != host->thread())
  {
    error = "Workspace attachment requires the Cubit GUI thread.";
    return false;
  }

  gui = host;
  commandDock = enclosingDock(host->command_line(), host);
  navigationDock = enclosingDock(host->command_buttons(), host);
  nativeNavigationDockVisible = navigationDock != nullptr && !navigationDock->isHidden();
  viewport = host->graphics_widget();
  menu = host->menuBar();
  nativeNavigation = host->navigation_model();
  navigationMode = host->command_buttons() == nullptr ? nullptr : host->command_buttons()->findChild<QGroupBox*>("mModeGroup");

  if (navigationMode != nullptr)
  {
    nativeModeVisible = !navigationMode->isHidden();
    navigationMode->installEventFilter(this);
  }

  if (!validate(error))
  {
    return false;
  }

  nativeSelection = host->command_buttons()->getCurrent();
  nativeLayout = host->saveState(1);
  bottomLeft = host->corner(Qt::BottomLeftCorner);
  bottomRight = host->corner(Qt::BottomRightCorner);

  for (auto* dock : host->findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly))
  {
    auto* action = dock->toggleViewAction();
    nativeDocks.append({dock, !dock->isHidden(), action->isVisible(), action->isEnabled()});
    watch_dock(dock);
  }

  for (auto* action : menu->actions())
  {
    nativeMenus.append({action, action->isVisible()});
    QString label = action->text();
    label.remove('&');
    if (action->menu() != nullptr && label.compare(tr("Tools"), Qt::CaseInsensitive) == 0)
    {
      // Borrow Cubit's existing actions, including its Options dialog handler.
      // Application workspaces share this menu rather than copying its entries.
      nativeToolsMenu = action;
    }
  }

  documentation = std::make_unique<DocumentationManager>();
  if (!documentation->attach(host, error))
  {
    return false;
  }

  // Power Tools can be siblings of the navigation page rather than a dock.
  for (auto* parent = host->command_buttons()->parentWidget(); parent != nullptr && parent != navigationDock; parent = parent->parentWidget())
  {
    if (auto* tabs = qobject_cast<QTabWidget*>(parent))
    {
      for (int index = 0; index < tabs->count(); ++index)
      {
        nativeTabs.append({tabs, tabs->widget(index), tabs->isTabVisible(index), tabs->currentIndex() == index});
      }
    }
  }

  titleSuffix = QString(" -- with Cubit-Sim %1").arg(CubitSim::Version);
  if (!gui->get_title().contains(titleSuffix))
  {
    gui->set_title(gui->get_title() + titleSuffix);
  }
  else
  {
    titleSuffix.clear();
  }
  return graphics->attach(viewport, error);
}

bool CubitWorkspaceHost::validate(std::string& error) const
{
  if (!can_access_ui() || commandDock == nullptr || navigationDock == nullptr || viewport == nullptr || menu == nullptr || nativeNavigation == nullptr)
  {
    error = "Cannot identify Cubit's viewport, command dock, navigation dock, menu or navigation model.";
    return false;
  }

  if (QThread::currentThread() != gui->thread() || gui->graphics_widget() != viewport || gui->command_buttons() == nullptr)
  {
    error = "The Cubit GUI host changed or switching was requested outside its GUI thread.";
    return false;
  }

  return true;
}

bool CubitWorkspaceHost::eventFilter(QObject* object, QEvent* event)
{
  if (object == navigationMode && event->type() == QEvent::Show && hideNavigationMode)
  {
    // Cubit may show its button groups again when another panel is selected.
    navigationMode->hide();
  }

  return QObject::eventFilter(object, event);
}

void CubitWorkspaceHost::watch_dock(QDockWidget* dock)
{
  connect(dock, &QDockWidget::visibilityChanged, this, [this, dock](bool visible)
  {
    if (!changing && visible && !dock_allowed(dock))
    {
      dock->hide();
    }
  });
}

bool CubitWorkspaceHost::dock_allowed(QDockWidget* dock) const
{
  if (dock == commandDock)
  {
    return true;
  }

  if (dock == navigationDock)
  {
    return useNativeNavigation;
  }

  for (const auto& owned : ownedDocks)
  {
    if (owned == dock)
    {
      return activeDocks.contains(owned);
    }
  }

  return nativeActive;
}

void CubitWorkspaceHost::place_workspace_toolbars(const WorkspaceView& view)
{
  // apply() guards these visibility changes with changing == true. Only module
  // toolbars may be removed/reinserted; native Cubit layout items stay intact.
  for (const auto& toolbar : ownedToolbars)
  {
    if (toolbar == nullptr)
    {
      continue;
    }

    toolbar->hide();

    if (gui->toolBarArea(toolbar) != Qt::NoToolBarArea)
    {
      gui->removeToolBarBreak(toolbar);
      gui->removeToolBar(toolbar);
    }
  }

  if (view.toolbars.isEmpty())
  {
    return;
  }

  bool nativeTopRowVisible = false;

  for (auto* toolbar : gui->findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly))
  {
    if (!ownedToolbars.contains(toolbar) &&
        gui->toolBarArea(toolbar) == Qt::TopToolBarArea &&
        !toolbar->isHidden())
    {
      nativeTopRowVisible = true;
      break;
    }
  }

  if (nativeTopRowVisible)
  {
    gui->addToolBarBreak(Qt::TopToolBarArea);
  }

  for (auto* toolbar : view.toolbars)
  {
    if (view.toolbarRowBreaks.contains(toolbar))
    {
      gui->addToolBarBreak(Qt::TopToolBarArea);
    }
    gui->addToolBar(Qt::TopToolBarArea, toolbar);
  }
}

void CubitWorkspaceHost::register_view(const WorkspaceView& view)
{
  for (auto* toolbar : view.toolbars)
  {
    ownedToolbars.append(toolbar);
    gui->addToolBar(Qt::TopToolBarArea, toolbar);
    toolbar->hide();
    const QString visibilityKey = "Workspaces/" + view.id + "/Toolbars/" + toolbar->objectName() + "/Visible";
    toolbarVisibility[toolbar] = QSettings("cubit-sim", "Framework").value(visibilityKey, true).toBool();
    connect(toolbar, &QToolBar::visibilityChanged, this, [this, toolbar, visibilityKey](bool)
    {
      if (!changing && activeToolbars.contains(toolbar) && gui != nullptr && gui->isVisible())
      {
        const bool visible = !toolbar->isHidden();
        toolbarVisibility[toolbar] = visible;
        QSettings("cubit-sim", "Framework").setValue(visibilityKey, visible);
      }
    });
    toolbar->toggleViewAction()->setVisible(false);
    toolbar->toggleViewAction()->setEnabled(false);
  }

  for (auto* dock : view.docks)
  {
    ownedDocks.append(dock);
    watch_dock(dock);
    dock->hide();
    dock->toggleViewAction()->setVisible(false);
    dock->toggleViewAction()->setEnabled(false);
  }

  for (auto* action : view.menuActions)
  {
    ownedMenus.append(action);
    action->setVisible(false);
  }
}

WorkspaceView CubitWorkspaceHost::native_view() const
{
  WorkspaceView view;
  view.id = "cubit";
  view.label = "Cubit";
  view.icon = gui->windowIcon();
  view.navigationModel = nativeNavigation.data();
  return view;
}

void CubitWorkspaceHost::remember_native()
{
  if (!nativeActive || gui == nullptr)
  {
    return;
  }

  nativeSelection = current_index();
  nativeNavigationDockVisible = !navigationDock->isHidden();

  if (navigationMode != nullptr)
  {
    nativeModeVisible = !navigationMode->isHidden();
  }

  for (auto& item : nativeMenus)
  {
    if (item.action != nullptr)
    {
      item.visible = item.action->isVisible();
    }
  }

  for (auto& item : nativeDocks)
  {
    if (item.widget != nullptr)
    {
      item.actionVisible = item.widget->toggleViewAction()->isVisible();
      item.actionEnabled = item.widget->toggleViewAction()->isEnabled();
    }
  }

  for (auto& item : nativeTabs)
  {
    if (item.tabs != nullptr && item.page != nullptr)
    {
      const int index = item.tabs->indexOf(item.page);

      if (index >= 0)
      {
        item.visible = item.tabs->isTabVisible(index);
        item.current = item.tabs->currentIndex() == index;
      }
    }
  }
}

bool CubitWorkspaceHost::can_access_ui() const
{
  return QCoreApplication::instance() != nullptr && !QCoreApplication::closingDown() && gui != nullptr && QThread::currentThread() == gui->thread() && gui->layout() != nullptr;
}

QByteArray CubitWorkspaceHost::save_layout() const
{
  return can_access_ui() ? gui->saveState(1) : QByteArray();
}

bool CubitWorkspaceHost::restore_layout(const QByteArray& layout)
{
  if (!can_access_ui() || layout.isEmpty())
  {
    return false;
  }

  const QScopedValueRollback<bool> guard(changing, true);
  struct ToolbarState
  {
    QPointer<QToolBar> bar;
    bool visible;
  };

  QList<ToolbarState> toolbars;

  for (auto* bar : gui->findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly))
  {
    if (!ownedToolbars.contains(bar))
    {
      toolbars.append({bar, !bar->isHidden()});
    }
  }

  const auto commandArea = gui->dockWidgetArea(commandDock);
  const bool commandFloating = commandDock->isFloating();
  const bool commandVisible = !commandDock->isHidden();
  const QRect commandGeometry = commandDock->geometry();
  const bool restored = gui->restoreState(layout, 1);

  // QMainWindow owns the toolbar layout items, including pending restore state.
  // Removing and re-adding native toolbars here can invalidate those items.
  for (const auto& item : toolbars)
  {
    if (item.bar != nullptr)
    {
      item.bar->setVisible(item.visible);
    }
  }

  if (commandArea != Qt::NoDockWidgetArea && gui->dockWidgetArea(commandDock) != commandArea)
  {
    gui->addDockWidget(commandArea, commandDock);
  }

  commandDock->setFloating(commandFloating);

  if (commandFloating)
  {
    commandDock->setGeometry(commandGeometry);
  }
  else
  {
    const bool horizontal = commandArea == Qt::BottomDockWidgetArea || commandArea == Qt::TopDockWidgetArea;
    gui->resizeDocks({commandDock.data()}, {horizontal ? commandGeometry.height() : commandGeometry.width()}, horizontal ? Qt::Vertical : Qt::Horizontal);
  }

  commandDock->setVisible(commandVisible);
  return restored;
}

bool CubitWorkspaceHost::apply(const WorkspaceView& view, const QPersistentModelIndex& selection, bool firstVisit, std::string& error)
{
  if (!validate(error) || (view.useNativeNavigation && view.navigationModel == nullptr))
  {
    if (error.empty())
    {
      error = "The requested workspace has no navigation model.";
    }

    return false;
  }

  const QScopedValueRollback<bool> guard(changing, true);
  if (!graphics->activate(view.graphicsPage, error))
  {
    return false;
  }

  const bool nextIndependent = view.graphicsPage != nullptr;

  if (nextIndependent && !independentGraphics)
  {
    commandAllowedAreas = commandDock->allowedAreas();
    commandFeatures = commandDock->features();
    commandArea = gui->dockWidgetArea(commandDock);
    commandFloating = commandDock->isFloating();
    commandGeometry = commandDock->geometry();
    nativeToolbarVisibility.clear();

    for (auto* toolbar : gui->findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly))
    {
      if (!ownedToolbars.contains(toolbar))
      {
        nativeToolbarVisibility.append({toolbar, !toolbar->isHidden()});
        toolbar->hide();
      }
    }
  }
  else if (!nextIndependent && independentGraphics)
  {
    commandDock->setAllowedAreas(commandAllowedAreas);
    commandDock->setFeatures(commandFeatures);

    if (commandArea != Qt::NoDockWidgetArea)
    {
      gui->addDockWidget(commandArea, commandDock);
    }

    commandDock->setFloating(commandFloating);

    if (commandFloating)
    {
      commandDock->setGeometry(commandGeometry);
    }

    for (const auto& item : nativeToolbarVisibility)
    {
      if (item.first != nullptr)
      {
        item.first->setVisible(item.second);
      }
    }
    nativeToolbarVisibility.clear();
  }

  independentGraphics = nextIndependent;
  useNativeNavigation = view.useNativeNavigation;

  if (independentGraphics)
  {
    commandDock->setFloating(false);
    commandDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    commandDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    gui->addDockWidget(Qt::BottomDockWidgetArea, commandDock);
  }
  commandDock->show();
  navigationDock->setVisible(useNativeNavigation && (view.id != "cubit" || nativeNavigationDockVisible));
  nativeActive = view.id == "cubit";
  activeDocks.clear();

  for (auto* dock : view.docks)
  {
    activeDocks.append(dock);
  }

  for (const auto& item : nativeDocks)
  {
    if (item.widget == nullptr || item.widget == commandDock || item.widget == navigationDock)
    {
      continue;
    }

    if (!nativeActive)
    {
      item.widget->hide();
    }

    item.widget->toggleViewAction()->setVisible(nativeActive && item.actionVisible);
    item.widget->toggleViewAction()->setEnabled(nativeActive && item.actionEnabled);
  }

  for (const auto& dock : ownedDocks)
  {
    if (dock == nullptr)
    {
      continue;
    }

    const bool active = activeDocks.contains(dock);

    if (!active)
    {
      dock->hide();
    }
    else if (firstVisit)
    {
      dock->show();
    }

    dock->toggleViewAction()->setVisible(active);
    dock->toggleViewAction()->setEnabled(active);
  }

  if (!nativeActive && firstVisit && view.docks.size() >= 2)
  {
    auto* modelDock = view.docks.at(0);
    auto* propertiesDock = view.docks.at(1);
    gui->addDockWidget(Qt::LeftDockWidgetArea, modelDock);
    if (view.dockLayout == WorkspaceDockLayout::LeftRight)
    {
      gui->addDockWidget(Qt::RightDockWidgetArea, propertiesDock);
      gui->resizeDocks({modelDock, propertiesDock}, {260, 280}, Qt::Horizontal);
    }
    else
    {
      gui->splitDockWidget(modelDock, propertiesDock, Qt::Vertical);
      gui->resizeDocks({modelDock, propertiesDock}, {420, 220}, Qt::Vertical);
      gui->resizeDocks({modelDock}, {300}, Qt::Horizontal);
    }
  }

  if ((firstVisit || view.initializeGraphicsBottomDock) && view.graphicsBottomDock != nullptr && independentGraphics)
  {
    view.graphicsBottomDock->setFloating(false);
    gui->addDockWidget(Qt::BottomDockWidgetArea,view.graphicsBottomDock);
    gui->splitDockWidget(view.graphicsBottomDock,commandDock,Qt::Vertical);
    view.graphicsBottomDock->show();
    gui->resizeDocks({view.graphicsBottomDock},{view.graphicsBottomDock->sizeHint().height()},Qt::Vertical);
  }

  for (const auto& item : nativeMenus)
  {
    if (item.action != nullptr)
    {
      item.action->setVisible(item.action == nativeToolsMenu || (nativeActive && item.visible));
    }
  }

  for (const auto& action : ownedMenus)
  {
    if (action != nullptr)
    {
      action->setVisible(view.menuActions.contains(action.data()));
    }
  }

  if (nativeToolsMenu != nullptr)
  {
    // Workspace menus are appended after the native menus. Move Tools behind
    // them, then let DocumentationManager place the active Help entry last.
    menu->removeAction(nativeToolsMenu.data());
    menu->addAction(nativeToolsMenu.data());
  }

  if (documentation != nullptr)
  {
    documentation->set_native_workspace(nativeActive);
  }

  activeToolbars.clear();
  for (auto* toolbar : view.toolbars)
  {
    activeToolbars.append(toolbar);
  }
  // Saved Qt layouts may have merged module bars into native rows. Reassert
  // the workspace group after restoration and before applying saved visibility.
  place_workspace_toolbars(view);
  for (const auto& toolbar : ownedToolbars)
  {
    if (toolbar == nullptr)
    {
      continue;
    }
    const bool active = activeToolbars.contains(toolbar);
    toolbar->setVisible(active && toolbarVisibility.value(toolbar.data(), true));
    toolbar->toggleViewAction()->setVisible(active);
    toolbar->toggleViewAction()->setEnabled(active);
  }

  auto* panel = gui->command_buttons();

  for (const auto& item : nativeTabs)
  {
    if (item.tabs == nullptr || item.page == nullptr)
    {
      continue;
    }

    const int index = item.tabs->indexOf(item.page);

    if (index >= 0)
    {
      const bool navigationPage = item.page == panel || item.page->isAncestorOf(panel);
      item.tabs->setTabVisible(index, nativeActive ? item.visible : (useNativeNavigation && navigationPage));

      if ((!nativeActive && useNativeNavigation && navigationPage) || (nativeActive && item.current && item.visible))
      {
        item.tabs->setCurrentIndex(index);
      }
    }
  }

  // Let Cubit construct the complete path before suppressing its Mode row.
  hideNavigationMode = false;
  if (useNativeNavigation)
  {
    panel->setModel(view.navigationModel, view.navigationRoot);

    if (selection.isValid() && selection.model() == view.navigationModel)
    {
      panel->setCurrent(selection);
    }
    else if (view.initialNavigationSelection.isValid() && view.initialNavigationSelection.model() == view.navigationModel)
    {
      panel->setCurrent(view.initialNavigationSelection);
    }
  }

  hideNavigationMode = !useNativeNavigation || view.hideNavigationMode;
  if (navigationMode != nullptr)
  {
    navigationMode->setVisible(!hideNavigationMode && (nativeActive ? nativeModeVisible : true));
  }

  gui->setCorner(Qt::BottomLeftCorner, nativeActive ? bottomLeft : Qt::LeftDockWidgetArea);
  gui->setCorner(Qt::BottomRightCorner, nativeActive ? bottomRight : Qt::RightDockWidgetArea);
  return true;
}

void CubitWorkspaceHost::restore_native() noexcept
{
  const QScopedValueRollback<bool> guard(changing, true);
  for (const auto& toolbar : ownedToolbars)
  {
    if (toolbar != nullptr)
    {
      toolbar->hide();
      toolbar->toggleViewAction()->setVisible(false);
      toolbar->toggleViewAction()->setEnabled(false);
    }
  }

  if (!can_access_ui())
  {
    return;
  }

  try
  {
    restore_layout(nativeLayout);
    std::string error;
    apply(native_view(), nativeSelection, false, error);
  }
  catch (...)
  {
    // Cleanup must not propagate an exception into Cubit's component loader.
  }
}

QPersistentModelIndex CubitWorkspaceHost::current_index() const
{
  return gui == nullptr || gui->command_buttons() == nullptr ? QPersistentModelIndex() : QPersistentModelIndex(gui->command_buttons()->getCurrent());
}

Claro* CubitWorkspaceHost::window() const
{
  return gui.data();
}

QMenuBar* CubitWorkspaceHost::menu_bar() const
{
  return menu.data();
}
