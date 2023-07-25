/*!
 *  \file MenuManager.hpp
 *  \brief
 *    Provides an example of how to create menus and menu items using
 *    the Claro framework.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#ifndef MENUMANAGER_HPP
#define MENUMANAGER_HPP

#include <QObject>

class MenuManager : public QObject
{
  Q_OBJECT

public:
  MenuManager(QObject* parent = 0);
  ~MenuManager();

  // Provides an example of adding an item to an exisiting menu
  void add_to_existing_menu();

  // Provides an example of creating your own menu.
  void setup_custom_menu();

  // Provides an example of how to remove your menu items.
  void remove_menu_items();

public slots:
  void myTool_triggered();
  void show_dialog_triggered();
  void something_cool_triggered();

private:
  bool existingInitialized;
  bool customInitialized;

};

#endif // MENUMANAGER_HPP
