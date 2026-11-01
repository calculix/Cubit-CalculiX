#ifndef WORKSPACEPROVIDERINTERFACE_HPP
#define WORKSPACEPROVIDERINTERFACE_HPP

#include <QIcon>
#include <QList>
#include <QPersistentModelIndex>
#include <QString>
#include <string>

class QAction;
class QDockWidget;
class NavigationModel;

struct WorkspaceView
{
  QString id;
  QString label;
  QIcon icon;
  QList<QDockWidget*> docks;
  QList<QAction*> menuActions;
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
