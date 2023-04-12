#include "CCXDockWindowMaterialManagement.hpp"
#include "Claro.hpp"
#include "ClaroViewMenu.hpp"
#include <iostream>

#include "ModelTree.hpp"
#include "BlocksTree.hpp"
#include "NodesetTree.hpp"
#include "SidesetTree.hpp"
#include "MaterialTree.hpp"
#include "SectionsTree.hpp"
#include "ConstraintsTree.hpp"
#include "SurfaceInteractionsTree.hpp"
#include "ContactPairsTree.hpp"
#include "AmplitudesTree.hpp"
#include "LoadsTree.hpp"
#include "LoadsForcesTree.hpp"
#include "LoadsPressuresTree.hpp"
#include "BCsTree.hpp"
#include "BCsDisplacementsTree.hpp"
#include "BCsTemperaturesTree.hpp"
#include "HistoryOutputsTree.hpp"
#include "FieldOutputsTree.hpp"
#include "InitialConditionsTree.hpp"
#include "StepsTree.hpp"
#include "JobsTree.hpp"


CCXDockWindowMaterialManagement::CCXDockWindowMaterialManagement() :
  isInitialized(false)
{ 
  dock_title = "CalculiX Material Management";
}

CCXDockWindowMaterialManagement::~CCXDockWindowMaterialManagement()
{
}

void CCXDockWindowMaterialManagement::initialize()
{
  if(isInitialized)
    return;

  gui = Claro::instance();

  ViewMenu = gui->view_menu();

  QString title;
  
  std::vector<QString> dock_labels;
  
  title = gui->get_title();
  title = title + " -- with CalculiX";
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

  dock = new QDockWidget(dock_title);
  dock->setAllowedAreas(Qt::NoDockWidgetArea);
  dock->setObjectName(dock_title);
  dock->resize(500, dock->height());
  dock->resize(800, dock->width());
  
  //dock->setTitleBarWidget(0);
  myModelTree = new ModelTree(dock);
  //myModelTree->header()->setMinimumSectionSize(25);
  //myModelTree->header()->resizeSection(1, 280);
  
  myBlocksTree = new BlocksTree(myModelTree);
  myBlocksTree->initialize();
  

  dock->setWidget(myModelTree);
      
  gui->add_dock_window(dock,"CalculiXComp",Qt::NoDockWidgetArea,Qt::NoDockWidgetArea);
    
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

void CCXDockWindowMaterialManagement::clear()
{
 // Remove all of our menu items.
  
  if(gui)
  { 
    // Items are removed based on the component name
    gui->remove_dock_windows("CalculiXComp");
    ViewMenu->remove_component_items("CalculiXComp");
    delete myModelTree;
    delete dock;
    isInitialized = false;  
  }
  
}

void CCXDockWindowMaterialManagement::update()
{
  settings->setValue("geometry",dock->saveGeometry());
  settings->setValue("state",gui->saveState());

  // update our dock items
  //myBlocksTree->update(); 
  
}

void CCXDockWindowMaterialManagement::reset()
{

  //myJobsTree->clear();
}