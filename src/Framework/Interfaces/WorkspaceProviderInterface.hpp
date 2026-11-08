#ifndef WORKSPACEPROVIDERINTERFACE_HPP
#define WORKSPACEPROVIDERINTERFACE_HPP

#include <QIcon>
#include <QList>
#include <QPersistentModelIndex>
#include <QString>
#include <string>
#include <functional>
#include <vector>

class QWidget;
class QAction;
class QToolBar;
class QDockWidget;
class NavigationModel;

enum class WorkspaceDockLayout
{
  StackedLeft,
  LeftRight
};

struct WorkspaceOptionsPage
{
  QString label;
  QWidget* widget = nullptr;
  std::function<void()> refresh;
  std::function<bool(std::string&)> apply;
};

struct WorkspaceView
{
  // Borrowed options widget; framework unregisters it before provider teardown.
  QWidget* optionsPage = nullptr;
  std::function<void()> refreshOptions;
  std::function<bool(std::string&)> applyOptions;
  // Same borrowed-widget lifetime as optionsPage; labels must be globally unique.
  std::vector<WorkspaceOptionsPage> additionalOptionsPages;
  QString id;
  QString label;
  QIcon icon;
  QList<QDockWidget*> docks;
  QList<QAction*> menuActions;
  // Ordered module toolbar group. The host places it together in the top area,
  // after one row break when native top toolbars are visible. Placement is
  // reasserted after layout restoration; per-toolbar visibility is preserved.
  QList<QToolBar*> toolbars;
  std::function<void(bool)> activationChanged;
  NavigationModel* navigationModel = nullptr;
  QPersistentModelIndex navigationRoot;
  QPersistentModelIndex initialNavigationSelection;
  bool hideNavigationMode = false;
  // Borrowed page; nullptr retains the original Cubit viewport.
  QWidget* graphicsPage = nullptr;
  bool useNativeNavigation = true;
  WorkspaceDockLayout dockLayout = WorkspaceDockLayout::StackedLeft;
};

class WorkspaceProviderInterface
{
public:
  virtual ~WorkspaceProviderInterface() = default;

  // The provider owns the widgets/model until its component is stopped.
  virtual bool workspace_view(WorkspaceView& view, std::string& error) = 0;
};

#endif // WORKSPACEPROVIDERINTERFACE_HPP
