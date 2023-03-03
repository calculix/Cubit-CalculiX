#include "CCXDockWindowTree.hpp"
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


CCXDockWindowTree::CCXDockWindowTree() :
  isInitialized(false)
{ 
  dock_title = "CalculiX Model Tree";
}

CCXDockWindowTree::~CCXDockWindowTree()
{
}

void CCXDockWindowTree::initialize()
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

  dock = new QDockWidget(dock_title,gui);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setObjectName(dock_title);
  myModelTree = new ModelTree(dock);
  myModelTree->header()->setMinimumSectionSize(25);
  myModelTree->header()->resizeSection(0, 180);
  myModelTree->header()->resizeSection(1, 25);
  
  myBlocksTree = new BlocksTree(myModelTree);
  myBlocksTree->initialize();
  myNodesetTree = new NodesetTree(myModelTree);
  myNodesetTree->initialize();
  mySidesetTree = new SidesetTree(myModelTree);
  mySidesetTree->initialize();
  myMaterialTree = new MaterialTree(myModelTree);
  myMaterialTree->initialize();   
  mySectionsTree = new SectionsTree(myModelTree);
  mySectionsTree->initialize();   
  myConstraintsTree = new ConstraintsTree(myModelTree);
  myConstraintsTree->initialize();   
  mySurfaceInteractionsTree = new SurfaceInteractionsTree(myModelTree);
  mySurfaceInteractionsTree->initialize();
  myContactPairsTree = new ContactPairsTree(myModelTree);
  myContactPairsTree->initialize();
  myAmplitudesTree = new AmplitudesTree(myModelTree);
  myAmplitudesTree->initialize();
  myLoadsTree = new LoadsTree(myModelTree);
  myLoadsTree->initialize();
  myLoadsForcesTree = new LoadsForcesTree(myLoadsTree);
  myLoadsForcesTree->initialize();
  myLoadsPressuresTree = new LoadsPressuresTree(myLoadsTree);
  myLoadsPressuresTree->initialize();
  myBCsTree = new BCsTree(myModelTree);
  myBCsTree->initialize();
  myBCsDisplacementsTree = new BCsDisplacementsTree(myBCsTree);
  myBCsDisplacementsTree->initialize();
  myBCsTemperaturesTree = new BCsTemperaturesTree(myBCsTree);
  myBCsTemperaturesTree->initialize();
  myHistoryOutputsTree = new HistoryOutputsTree(myModelTree);
  myHistoryOutputsTree->initialize();
  myFieldOutputsTree = new FieldOutputsTree(myModelTree);
  myFieldOutputsTree->initialize();
  myInitialConditionsTree = new InitialConditionsTree(myModelTree);
  myInitialConditionsTree->initialize();
  myStepsTree = new StepsTree(myModelTree);
  myStepsTree->initialize();

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

void CCXDockWindowTree::clear()
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
    delete myMaterialTree;
    delete mySectionsTree;
    delete myConstraintsTree;
    delete mySurfaceInteractionsTree;
    delete myContactPairsTree;
    delete myAmplitudesTree;
    delete myLoadsForcesTree;
    delete myLoadsPressuresTree;
    delete myLoadsTree;
    delete myBCsTemperaturesTree;
    delete myBCsDisplacementsTree;
    delete myBCsTree;
    delete myHistoryOutputsTree;
    delete myFieldOutputsTree;
    delete myInitialConditionsTree;
    delete myStepsTree;
    delete myModelTree;
    delete dock;
    isInitialized = false;  
  }
  
}

void CCXDockWindowTree::update()
{
  settings->setValue("geometry",dock->saveGeometry());
  settings->setValue("state",gui->saveState());

  // update our dock items
  myBlocksTree->update(); 
  myNodesetTree->update(); 
  mySidesetTree->update(); 
  myMaterialTree->update(); 
  mySectionsTree->update();
  myConstraintsTree->update();
  mySurfaceInteractionsTree->update();
  myContactPairsTree->update();
  myAmplitudesTree->update();
  //myLoadsTree->update();
  myLoadsForcesTree->update();
  myLoadsPressuresTree->update();
  //myBCsTree->update();
  myBCsDisplacementsTree->update();
  myBCsTemperaturesTree->update();
  myHistoryOutputsTree->update();
  myFieldOutputsTree->update();
  myInitialConditionsTree->update();
  myStepsTree->update();
}

void CCXDockWindowTree::reset()
{
  // reset, our trees, so clear all child items
  myBlocksTree->clear(); 
  myNodesetTree->clear(); 
  mySidesetTree->clear(); 
  myMaterialTree->clear(); 
  mySectionsTree->clear();
  myConstraintsTree->clear();
  mySurfaceInteractionsTree->clear();
  myContactPairsTree->clear();
  myAmplitudesTree->clear();
  //myLoadsTree->clear();
  myLoadsForcesTree->clear();
  myLoadsPressuresTree->clear();
  //myBCsTree->clear();
  myBCsDisplacementsTree->clear();
  myBCsTemperaturesTree->clear();
  myHistoryOutputsTree->clear();
  myFieldOutputsTree->clear();
  myInitialConditionsTree->clear();
  myStepsTree->clear();
}