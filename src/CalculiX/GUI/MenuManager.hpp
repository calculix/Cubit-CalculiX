/*! @file MenuManager.hpp
 *  @brief Provides the Cubit-CalculiX documentation entry in the Help menu.
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

class MenuManager : public QObject
{
  Q_OBJECT

public:
  MenuManager(QObject* parent = nullptr);
  ~MenuManager();

  void setup_workspace_menus(Claro* host, QDockWidget* modelDock, QDockWidget* propertiesDock);
  QList<QAction*> workspace_menu_actions() const;
  void remove_menu_items();

public slots:
  void open_documentation();

private:
  QList<QPointer<QMenu>> menus;
};

#endif // MENUMANAGER_HPP
