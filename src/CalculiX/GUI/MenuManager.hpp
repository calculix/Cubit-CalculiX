/*! @file MenuManager.hpp
 *  @brief Provides CalculiX workspace File, View and Options menus.
 */
#ifndef MENUMANAGER_HPP
#define MENUMANAGER_HPP

#include <QObject>
#include <QPointer>
#include <QList>

class QAction;
class Claro;
class QDockWidget;
class QMenu;
class QToolBar;

class MenuManager : public QObject
{
  Q_OBJECT

public:
  MenuManager(QObject* parent = nullptr);
  ~MenuManager();

  void setup_workspace_menus(Claro* host, QDockWidget* modelDock, QDockWidget* propertiesDock, QToolBar* drawingToolbar, QToolBar* topologyToolbar);
  QList<QAction*> workspace_menu_actions() const;
  void remove_menu_items();

private:
  QList<QPointer<QMenu>> menus;
};

#endif // MENUMANAGER_HPP
