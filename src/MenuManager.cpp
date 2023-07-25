/*!
 *  \file MenuManager.cpp
 *  \brief
 *    Provides an example of how to create menus and menu items using
 *    the Claro framework.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#include "MenuManager.hpp"
#include "Claro.hpp"
#include "CommandWindow.hpp"

#include <QAction>
#include <vector>

MenuManager::MenuManager(QObject *parent) :
  QObject(parent),
  existingInitialized(false),
  customInitialized(false)
{}

MenuManager::~MenuManager()
{}

// This will function will add an item labeled "MyTool" in the Tools menu.
void MenuManager::add_to_existing_menu()
{
  if(existingInitialized)
    return;

  std::vector<QAction*> menu_list;

  // Create the QAction that represents our tool and connect it to the
  // appropriate slot.
  QAction* action = new QAction(this);
  action->setText(tr("MyTool"));
  connect(action, SIGNAL(triggered()),
          this, SLOT(myTool_triggered()));

  menu_list.push_back(action);

  Claro* gui = Claro::instance();
  if(gui)
  {
    // Add the item to the Tools menu.
    gui->add_to_menu(tr("&Tools"), menu_list, "mycomp");

    existingInitialized = true;
  }
  else
    delete action;
}

// This function will create the "MyMenu" menu, with a list of actions that
// are useful for MyComp.
void MenuManager::setup_custom_menu()
{
  if(customInitialized)
    return;

  std::vector<QAction*> menu_list;

  // Create the QActions and connect them to the appropriate slots.
  QAction* action = new QAction(this);
  action->setText(tr("Show Dialog"));
  connect(action, SIGNAL(triggered()),
          this, SLOT(show_dialog_triggered()));

  menu_list.push_back(action);

  // TODO: Change the name of this action and the name of it's slot.
  action = new QAction(this);
  action->setText(tr("Something Cool"));
  connect(action, SIGNAL(triggered()),
          this, SLOT(something_cool_triggered()));

  menu_list.push_back(action);

  Claro* gui = Claro::instance();
  if(gui)
  {
    // Add the item to our menu. Note that if the named menu does not exist,
    // Claro will create it for us.
    gui->add_to_menu(tr("&MyMenu"), menu_list, "mycomp");

    customInitialized = true;
  }
  else
  {
    for(size_t i = 0; i < menu_list.size(); i++)
      delete menu_list[i];
  }
}

void MenuManager::remove_menu_items()
{
  // Remove all of our menu items.
  Claro* gui = Claro::instance();
  if(gui)
  {
    // Items are removed based on the component name
    gui->remove_menu_items("mycomp");

    existingInitialized = false;
    customInitialized = false;
  }
}

void MenuManager::myTool_triggered()
{
  // TODO: Something more demonstrative that printing to the
  // command window.
  CommandWindow* cw = Claro::instance()->command_line();

  cw->print_data("MyTool was triggered\n");
}

void MenuManager::show_dialog_triggered()
{
  // TODO: Something more demonstrative that printing to the
  // command window.
  CommandWindow* cw = Claro::instance()->command_line();

  cw->print_data("ShowDialog was triggered\n");
}

void MenuManager::something_cool_triggered()
{
  // TODO: Something more demonstrative that printing to the
  // command window.
  CommandWindow* cw = Claro::instance()->command_line();

  cw->print_data("Something Cool was triggered\n");
}
