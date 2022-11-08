/*!
 *  \file ToolbarManager.cpp
 *  \brief
 *    Provides an example of how to create a toolbar and toolbar items
 *    using the Claro framework.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#include "ToolbarManager.hpp"
#include "CalculiXComp.hpp"
#include "Claro.hpp"
#include "CommandWindow.hpp"

#include <QAction>
#include <vector>

ToolbarManager::ToolbarManager(QObject *parent) :
  QObject(parent),
  existingInitialized(false),
  customInitialized(false)
{}

ToolbarManager::~ToolbarManager()
{}

void ToolbarManager::add_to_existing_toolbar()
{
  if(existingInitialized)
    return;

  std::vector<QAction*> tool_list;

  // Create the QAction that represents our tool and connect it
  // to an appropriate slot.
  QAction* action = new QAction(this);
  action->setText("TopView");
  connect(action, SIGNAL(triggered()), this, SLOT(topview_triggered()));

  tool_list.push_back(action);

  Claro* gui = Claro::instance();
  if(gui)
  {
    // Add the item to the toolbar.
    gui->add_to_toolbar("DisplayTools", tr("Display Tools"),
                        tool_list, "mycomp");

    existingInitialized = true;
  }
  else
    delete action;
}

void ToolbarManager::setup_custom_toolbar()
{
  if(customInitialized)
    return;

  std::vector<QAction*> tool_list;

  // Create QActions and connect them to the appropriate slots.
  QAction* action = new QAction(this);
  action->setText(tr("Button1"));
  connect(action, SIGNAL(triggered()), this, SLOT(button1_triggered()));

  tool_list.push_back(action);

  action = new QAction(this);
  action->setText(tr("Button2"));
  connect(action, SIGNAL(triggered()), this, SLOT(button2_triggered()));

  tool_list.push_back(action);

  Claro* gui = Claro::instance();
  if(gui)
  {
    gui->add_to_toolbar("MyToolbar", tr("MyToolbar"),
                        tool_list, "mycomp");

    customInitialized = true;
  }
  else
  {
    for(size_t i = 0; i < tool_list.size(); i++)
      delete tool_list[i];
  }
}

void ToolbarManager::remove_toolbar_items()
{
  // Remove all of our toolbar items.
  Claro* gui = Claro::instance();
  if(gui)
  {
    // Items are removed based on the component name used to create them
    gui->remove_toolbar_items("mycomp");

    existingInitialized = false;
    customInitialized = false;
  }
}

void ToolbarManager::topview_triggered()
{
  // TODO: Something more demonstrative that printing to the
  // command window.
  CommandWindow* cw = Claro::instance()->command_line();

  cw->print_data("TopView button was triggered\n");
}

void ToolbarManager::button1_triggered()
{
  // TODO: Something more demonstrative that printing to the
  // command window.
  CommandWindow* cw = Claro::instance()->command_line();

  cw->print_data("Button1 was triggered\n");
}

void ToolbarManager::button2_triggered()
{
  // TODO: Something more demonstrative that printing to the
  // command window.
  CommandWindow* cw = Claro::instance()->command_line();

  cw->print_data("Button2 was triggered\n");
}
