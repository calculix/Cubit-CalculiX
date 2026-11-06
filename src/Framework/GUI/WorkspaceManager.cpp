#include <QToolBar>
#include "WorkspaceManager.hpp"
#include "Framework/Interfaces/DrawingInterface.hpp"
#include "CubitWorkspaceHost.hpp"
#include "WorkspaceHeader.hpp"
#include "CommandDispatcher.hpp"
#include "Claro.hpp"
#include "CubitMessage.hpp"
#include "NavigationModel.hpp"
#include "Framework/Utility/StartupTrace.hpp"

#include <QAction>
#include <QCoreApplication>
#include <QDockWidget>
#include <QEvent>
#include <QScopedValueRollback>
#include <QSettings>
#include <QThread>
#include <exception>

namespace
{
class UpdateGuard
{
public:
  explicit UpdateGuard(QWidget* widget) : widget(widget), enabled(widget->updatesEnabled())
  {
    widget->setUpdatesEnabled(false);
  }

  ~UpdateGuard()
  {
    if (widget != nullptr)
    {
      widget->setUpdatesEnabled(enabled);
    }
  }

private:
  QPointer<QWidget> widget;
  bool enabled;
};
}

WorkspaceManager::WorkspaceManager(DrawingInterface& service) : drawings(service), host(std::make_unique<CubitWorkspaceHost>())
{
}

WorkspaceManager::~WorkspaceManager() = default;

bool WorkspaceManager::attach(QWidget* widget, std::string& error)
{
  if (!host->attach(qobject_cast<Claro*>(widget), error))
  {
    return false;
  }

  widget->installEventFilter(this);
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [this]()
  {
    startup_trace("Application aboutToQuit");
    shutdown();
  });
  connect(widget, &QObject::destroyed, this, [this]()
  {
    startup_trace("Workspace host destroyed");
    ready = false;
    stopped = true;
  });
  return true;
}

bool WorkspaceManager::register_views(const std::vector<WorkspaceProviderInterface*>& providers, std::string& error)
{
  if (ready || !host->validate(error))
  {
    if (error.empty())
    {
      error = "Workspaces have already been registered.";
    }

    return false;
  }

  views.insert("cubit", host->native_view());
  QList<QDockWidget*> registeredDocks;
  QList<QToolBar*> registeredToolbars;

  for (auto* provider : providers)
  {
    WorkspaceView view;

    if (provider == nullptr || !provider->workspace_view(view, error))
    {
      return false;
    }

    if (view.id.isEmpty() || view.id.contains(' ') || views.contains(view.id) || (view.useNativeNavigation && view.navigationModel == nullptr))
    {
      error = "A workspace has an invalid/duplicate identifier or no navigation model.";
      return false;
    }

    for (auto* dock : view.docks)
    {
      if (dock == nullptr || dock->objectName().isEmpty() || registeredDocks.contains(dock))
      {
        error = "Workspace docks must be named, non-null and owned by only one workspace.";
        return false;
      }

      registeredDocks.append(dock);
    }

    for (auto* action : view.menuActions)
    {
      if (action == nullptr)
      {
        error = "A workspace supplied a null menu action.";
        return false;
      }
    }

    for (auto* toolbar : view.toolbars)
    {
      if (toolbar == nullptr || toolbar->objectName().isEmpty() || registeredToolbars.contains(toolbar))
      {
        error = "Workspace toolbars must be named, non-null and owned by only one workspace.";
        return false;
      }
      registeredToolbars.append(toolbar);
    }

    views.insert(view.id, view);
  }

  QSettings settings("cubit-sim", "Framework");
  settings.beginGroup("Workspaces");

  for (auto it = views.cbegin(); it != views.cend(); ++it)
  {
    if (it.key() != "cubit")
    {
      host->register_view(it.value());

      if (settings.value("LayoutVersion", 1).toInt() == 1)
      {
        layouts[it.key()] = settings.value(it.key() + "/Layout").toByteArray();
      }
    }
  }

  // Undo any docking side effects from registering hidden module docks.
  host->restore_native();
  layouts["cubit"] = host->save_layout();
  selections["cubit"] = host->current_index();

  if (!host->apply(views.value("cubit"), selections.value("cubit"), false, error))
  {
    return false;
  }

  header = new WorkspaceHeader(host->window(), [this](const QString& id)
  {
    std::string error;

    if (!CommandDispatcher::submit(host->window(), "sim workspace " + id, error))
    {
      PRINT_ERROR("Unable to submit workspace command: %s\n", error.c_str());
    }
  });

  header->add_workspace("cubit", "Cubit", views.value("cubit").icon);

  for (auto* provider : providers)
  {
    WorkspaceView view;

    if (!provider->workspace_view(view, error))
    {
      return false;
    }

    header->add_workspace(view.id, view.label, view.icon);
  }

  header->set_current("cubit");
  ready = true;
  return true;
}

bool WorkspaceManager::activate(const QString& id, std::string& error)
{
  startup_trace("Workspace requested: " + id);
  error.clear();

  if (!ready || switching || !host->validate(error))
  {
    if (error.empty())
    {
      error = "The workspace host is not ready or a switch is already in progress.";
    }

    return false;
  }

  if (!views.contains(id))
  {
    error = "Workspace is unavailable: " + id.toStdString();
    return false;
  }

  if (id == current)
  {
    return true;
  }

  const QScopedValueRollback<bool> guard(switching, true);
  const UpdateGuard updates(host->window());
  const QString previous = current;
  host->remember_native();
  const QByteArray before = host->save_layout();
  const QPersistentModelIndex previousSelection = host->current_index();
  layouts[previous] = before;
  selections[previous] = previousSelection;

  try
  {
    bool firstVisit = layouts.value(id).isEmpty();

    if (!firstVisit && !host->restore_layout(layouts.value(id)))
    {
      host->restore_layout(before);
      firstVisit = true;
    }

    if (!host->apply(views.value(id), selections.value(id), firstVisit, error))
    {
      host->restore_layout(before);
      std::string rollbackError;
      host->apply(views.value(previous), previousSelection, false, rollbackError);
      return false;
    }

    if (!drawings.activate_workspace(id.toStdString(), error))
    {
      host->restore_layout(before);
      std::string rollbackError;
      host->apply(views.value(previous), previousSelection, false, rollbackError);
      return false;
    }
    if (views.value(previous).activationChanged)
    {
      views.value(previous).activationChanged(false);
    }
    if (views.value(id).activationChanged)
    {
      views.value(id).activationChanged(true);
    }
    current = id;
    startup_trace("Workspace active: " + id);
    header->set_current(id);

    QSettings settings("cubit-sim", "Framework");
    settings.beginGroup("Workspaces");
    settings.setValue("LayoutVersion", 1);
    settings.setValue(previous + "/Layout", before);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Workspace switch failed: ") + exception.what();
  }
  catch (...)
  {
    error = "Workspace switch failed with an unknown exception.";
  }

  host->restore_layout(before);
  std::string rollbackError;
  host->apply(views.value(previous), previousSelection, false, rollbackError);
  try
  {
    if (views.value(id).activationChanged)
    {
      views.value(id).activationChanged(false);
    }
    std::string drawingRollback;
    if (!drawings.activate_workspace(previous.toStdString(), drawingRollback))
    {
      PRINT_ERROR("Cannot restore workspace drawings: %s\n", drawingRollback.c_str());
    }
    if (views.value(previous).activationChanged)
    {
      views.value(previous).activationChanged(true);
    }
  }
  catch (...)
  {
    PRINT_ERROR("Workspace graphics state could not be restored.\n");
  }
  current = previous;

  if (header != nullptr)
  {
    header->set_current(previous);
  }

  return false;
}

void WorkspaceManager::shutdown() noexcept
{
  if (stopped)
  {
    return;
  }

  stopped = true;
  startup_trace("Workspace shutdown");

  if (!host->can_access_ui())
  {
    if (views.value(current).activationChanged)
    {
      try
      {
        views.value(current).activationChanged(false);
      }
      catch (...)
      {
      }
    }
    ready = false;
    return;
  }

  try
  {
    if (ready && host->window() != nullptr)
    {
      QSettings settings("cubit-sim", "Framework");
      settings.beginGroup("Workspaces");
      settings.setValue("LayoutVersion", 1);
      settings.setValue(current + "/Layout", host->save_layout());
      std::string error;

      if (!activate("cubit", error))
      {
        PRINT_ERROR("Cannot restore the Cubit workspace during cleanup: %s\n", error.c_str());
        host->restore_native();
      }
    }
    else
    {
      host->restore_native();
    }

    ready = false;

    if (host->window() != nullptr)
    {
      host->window()->removeEventFilter(this);
    }

    delete header.data();
    header = nullptr;
    views.clear();
  }
  catch (...)
  {
    PRINT_ERROR("Workspace cleanup failed.\n");
  }
}

bool WorkspaceManager::eventFilter(QObject* object, QEvent* event)
{
  if (object == host->window() && event->type() == QEvent::Close && ready && !switching)
  {
    // Claro may save its settings in closeEvent, before component cleanup.
    std::string error;

    if (!activate("cubit", error))
    {
      PRINT_ERROR("Cannot restore the Cubit layout before closing: %s\n", error.c_str());
      host->restore_native();
    }
  }

  return QObject::eventFilter(object, event);
}
