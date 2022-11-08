/*!
 *  \file MyComp.cpp
 *  \brief
 *    Provides an example of creating a custom component.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#include "CalculiXComp.hpp"

#include "Broker.hpp"
#include "ComponentInfo.hpp"
#include "ExportManager.hpp"
#include "MenuManager.hpp"
#include "ObserverComp.hpp"
#include "PanelManager.hpp"
#include "ToolbarManager.hpp"

// Default constructor. Remember to include the component name (should match
// the module name in mycomp.i).
CalculiXComp::CalculiXComp() :
  Component("CalculiXComp"),
  myMenus(NULL),
  myToolbars(NULL),
  myPanels(NULL),
  myExportManager(NULL),
  mListener(NULL)
{}

CalculiXComp::~CalculiXComp()
{
  if(myMenus)
    delete myMenus;

  if(myToolbars)
    delete myToolbars;

  if(myPanels)
    delete myPanels;

  if(myExportManager)
    delete myExportManager;

  if(mListener)
    delete mListener;
}

void CalculiXComp::start_up(int withGUI)
{
  if(withGUI)
  {
    setup_menus();
    setup_toolbars();
    setup_command_panels();
    add_exports();
  }

  setup_observers(withGUI);
}

void CalculiXComp::clean_up()
{
  cleanup_menus();
  cleanup_toolbars();
  cleanup_command_panels();
  cleanup_exports();
  cleanup_observers();

  // Let the framework know you are done.
  clean_up_complete();
}

void CalculiXComp::setup_menus()
{
  if(!myMenus)
    myMenus = new MenuManager;

  myMenus->add_to_existing_menu();
  myMenus->setup_custom_menu();
}

void CalculiXComp::cleanup_menus()
{
  if(myMenus)
    myMenus->remove_menu_items();
}

void CalculiXComp::setup_toolbars()
{
  if(!myToolbars)
    myToolbars = new ToolbarManager;

  myToolbars->add_to_existing_toolbar();
  myToolbars->setup_custom_toolbar();
}

void CalculiXComp::cleanup_toolbars()
{
  if(myToolbars)
    myToolbars->remove_toolbar_items();
}

void CalculiXComp::setup_command_panels()
{
  if(!myPanels)
    myPanels = new PanelManager;

  myPanels->initialize();
}

void CalculiXComp::cleanup_command_panels()
{
  if(myPanels)
    myPanels->clear();
}

void CalculiXComp::add_exports()
{
  if(!myExportManager)
  {
    myExportManager = new ExportManager();
    myExportManager->add_export_types();
  }
}

void CalculiXComp::cleanup_exports()
{
  if(myExportManager)
    myExportManager->remove_export_types();
}

void CalculiXComp::setup_observers(int withGUI)
{
  if(!mListener)
  {
    mListener = new ObserverComp();

    // Let Cubit know that this class will be observing events
    mListener->register_observer();
  }
}

void CalculiXComp::cleanup_observers()
{
  if(mListener)
  {
    // If cubitcomp is still loaded, we need to unregister the observer.
    ComponentInfo cubitcomp_info;
    Broker::instance()->get_component_info("cubitcomp", cubitcomp_info);
    if(cubitcomp_info.get_state() == "Loaded")
      mListener->unregister_observer();

    delete mListener;
    mListener = NULL;
  }
}
