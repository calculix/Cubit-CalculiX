#ifndef CUBITWORKSPACEHOST_HPP
#define CUBITWORKSPACEHOST_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QPointer>
#include <QRect>

class QToolBar;
class Claro;
class QMenuBar;
class QGroupBox;
class QTabWidget;
class TaskPanel;
class QWidget;

class CubitWorkspaceHost : public QObject
{
public:
  bool attach(Claro* host, std::string& error);
  bool validate(std::string& error) const;
  bool can_access_ui() const;
  void register_view(const WorkspaceView& view);
  WorkspaceView native_view() const;
  void remember_native();
  QByteArray save_layout() const;
  bool restore_layout(const QByteArray& layout);
  bool apply(const WorkspaceView& view, const QPersistentModelIndex& selection, bool firstVisit, std::string& error);
  void restore_native() noexcept;
  QPersistentModelIndex current_index() const;
  Claro* window() const;
  QMenuBar* menu_bar() const;

protected:
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  struct DockState
  {
    QPointer<QDockWidget> widget;
    bool visible = false;
    bool actionVisible = false;
    bool actionEnabled = false;
  };

  struct MenuState
  {
    QPointer<QAction> action;
    bool visible = false;
  };

  struct TabState
  {
    QPointer<QTabWidget> tabs;
    QPointer<QWidget> page;
    bool visible = false;
    bool current = false;
  };

  void watch_dock(QDockWidget* dock);
  bool dock_allowed(QDockWidget* dock) const;
  QPointer<Claro> gui;
  QPointer<QDockWidget> commandDock;
  QPointer<QDockWidget> navigationDock;
  QPointer<QWidget> viewport;
  QPointer<QMenuBar> menu;
  QPointer<NavigationModel> nativeNavigation;
  QPointer<QGroupBox> navigationMode;
  QPersistentModelIndex nativeSelection;
  QList<DockState> nativeDocks;
  QList<MenuState> nativeMenus;
  QList<TabState> nativeTabs;
  QList<QPointer<QDockWidget>> ownedDocks;
  QList<QPointer<QAction>> ownedMenus;
  QMap<QToolBar*, bool> toolbarVisibility;
  QList<QPointer<QToolBar>> ownedToolbars;
  QList<QPointer<QToolBar>> activeToolbars;
  QList<QPointer<QDockWidget>> activeDocks;
  QByteArray nativeLayout;
  Qt::DockWidgetArea bottomLeft = Qt::LeftDockWidgetArea;
  Qt::DockWidgetArea bottomRight = Qt::RightDockWidgetArea;
  bool nativeActive = true;
  bool changing = false;
  bool nativeModeVisible = true;
  bool hideNavigationMode = false;
};

#endif // CUBITWORKSPACEHOST_HPP
