
#include "CCXDockWindow.hpp"
#include "Claro.hpp"
#include "ClaroViewMenu.hpp"
#include <iostream>
#include "ModelTree.hpp"
#include "BlocksTree.hpp"
#include "NodesetTree.hpp"
#include "SidesetTree.hpp"



CCXDockWindow::CCXDockWindow() :
  isInitialized(false)
{ 
  dock_title = "CalculiX Plugin";
}

CCXDockWindow::~CCXDockWindow()
{
}

void CCXDockWindow::initialize()
{
  if(isInitialized)
    return;

  gui = Claro::instance();

  ViewMenu = gui->view_menu();

  QString title;
  
  std::vector<QString> dock_labels;
  
  title = gui->get_title();
  title = title + " -- with CalculiX Plugin";
  std::cout << title.toStdString();
  
  gui->set_title(title);

  ViewMenu->get_dock_labels(dock_labels);
  std::cout << "dock labels \n";
  for (size_t i = 0; i < dock_labels.size(); i++)
  {
    std::cout << "---";
    std::cout << dock_labels[i].toStdString();
    std::cout << "\n";
  } 

  dock = new QDockWidget(dock_title,gui);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setObjectName(dock_title);
  myModelTree = new ModelTree(dock);
  myBlocksTree = new BlocksTree(myModelTree);
  myBlocksTree->initialize();
  myNodesetTree = new NodesetTree(myModelTree);
  myNodesetTree->initialize();
  mySidesetTree = new SidesetTree(myModelTree);
  mySidesetTree->initialize();
    
  dock->setWidget(myModelTree);
      
  gui->add_dock_window(dock,"CalculiXComp",Qt::LeftDockWidgetArea,Qt::AllDockWidgetAreas);
    
  ViewMenu->add_dock_item(dock->toggleViewAction(),"CalculiXComp");
  std::cout << "dock labels 2 \n";
  ViewMenu->get_dock_labels(dock_labels);
  for (size_t i = 0; i < dock_labels.size(); i++)
  {
    std::cout << "---";
    std::cout << dock_labels[i].toStdString();
    std::cout << "\n";
  }

  settings = new QSettings("CalculiXPlugin","CalculiXComp");
  dock->restoreGeometry(settings->value("geometry").toByteArray());
  gui->restoreState(settings->value("state").toByteArray());
    
  isInitialized = true;
}

void CCXDockWindow::clear()
{
 // Remove all of our menu items.
  
  if(gui)
  { 
    // Items are removed based on the component name
    gui->remove_dock_windows("CalculiXComp");
    ViewMenu->remove_component_items("CalculiXComp");
    delete myBlocksTree;
    delete myNodesetTree;
    delete mySidesetTree;
    delete myModelTree;
    delete dock;
    isInitialized = false;  
  }
  
}

void CCXDockWindow::update()
{
  settings->setValue("geometry",dock->saveGeometry());
  settings->setValue("state",gui->saveState());

  // update our dock items
  myBlocksTree->update(); 
  myNodesetTree->update(); 
  mySidesetTree->update(); 
}

void CCXDockWindow::reset()
{
  // reset, not necessary right now
}