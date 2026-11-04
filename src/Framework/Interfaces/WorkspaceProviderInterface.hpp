#ifndef WORKSPACEPROVIDERINTERFACE_HPP
#define WORKSPACEPROVIDERINTERFACE_HPP

#include <QIcon>
#include <QList>
#include <QPersistentModelIndex>
#include <QString>
#include <string>
#include <functional>

class QWidget;
class QAction;
class QToolBar;
class QDockWidget;
class NavigationModel;

struct WorkspaceView
{
  // Borrowed options widget; framework unregisters it before provider teardown.
  QWidget* optionsPage = nullptr;
  std::function<void()> refreshOptions;
  std::function<bool(std::string&)> applyOptions;
  QString id;
  QString label;
  QIcon icon;
  QList<QDockWidget*> docks;
  QList<QAction*> menuActions;
  QList<QToolBar*> toolbars;
  std::function<void(bool)> activationChanged;
  NavigationModel* navigationModel = nullptr;
  QPersistentModelIndex navigationRoot;
  QPersistentModelIndex initialNavigationSelection;
  bool hideNavigationMode = false;
};

class WorkspaceProviderInterface
{
public:
  virtual ~WorkspaceProviderInterface() = default;

  // The provider owns the widgets/model until its component is stopped.
  virtual bool workspace_view(WorkspaceView& view, std::string& error) = 0;
};

#endif // WORKSPACEPROVIDERINTERFACE_HPP
