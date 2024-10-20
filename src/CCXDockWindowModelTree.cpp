#include "CCXDockWindowModelTree.hpp"
#include "Claro.hpp"
#include "ClaroViewMenu.hpp"
#include <iostream>

#include "CalculiXCoreInterface.hpp"

#include "ModelTree.hpp"
#include "Highlight.hpp"
#include "BlocksTree.hpp"
#include "NodesetTree.hpp"
#include "SidesetTree.hpp"
#include "MaterialTree.hpp"
#include "SectionsTree.hpp"
#include "ConstraintsTree.hpp"
#include "SurfaceInteractionsTree.hpp"
#include "ContactPairsTree.hpp"
#include "AmplitudesTree.hpp"
#include "OrientationsTree.hpp"
#include "LoadsTree.hpp"
#include "LoadsForcesTree.hpp"
#include "LoadsPressuresTree.hpp"
#include "LoadsHeatfluxesTree.hpp"
#include "LoadsGravityTree.hpp"
#include "LoadsCentrifugalTree.hpp"
#include "BCsTree.hpp"
#include "BCsDisplacementsTree.hpp"
#include "BCsTemperaturesTree.hpp"
#include "HistoryOutputsTree.hpp"
#include "FieldOutputsTree.hpp"
#include "InitialConditionsTree.hpp"
#include "HBCsTree.hpp"
#include "HBCsDisplacementsTree.hpp"
#include "HBCsTemperaturesTree.hpp"
#include "StepsTree.hpp"
#include "CustomLinesTree.hpp"
#include "JobsTree.hpp"


CCXDockWindowModelTree::CCXDockWindowModelTree() :
  isInitialized(false)
{ 
  dock_title = "CalculiX Model Tree";
}

CCXDockWindowModelTree::~CCXDockWindowModelTree()
{
}

void CCXDockWindowModelTree::initialize()
{
  if(isInitialized)
    return;

  gui = Claro::instance();

  ViewMenu = gui->view_menu();

  QString title;
  
  //std::vector<QString> dock_labels;

  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  title = gui->get_title();
  title = title + " -- with Cubit-CalculiX " + QString::fromStdString(ccx_iface->get_version());
  //std::cout << title.toStdString();
  
  gui->set_title(title);

  /*
  ViewMenu->get_dock_labels(dock_labels);
  std::cout << "dock labels \n";
  for (size_t i = 0; i < dock_labels.size(); i++)
  {
    std::cout << "---";
    std::cout << dock_labels[i].toStdString();
    std::cout << "\n";
  } 
  */
  dock = new QDockWidget(dock_title,gui);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setObjectName(dock_title);
  myModelTree = new ModelTree(dock);
  myModelTree->header()->setMinimumSectionSize(25);
  myModelTree->header()->resizeSection(0, 180);
  myModelTree->header()->resizeSection(1, 25);
  myHighlight = new Highlight(myModelTree);
  
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
  myOrientationsTree = new OrientationsTree(myModelTree);
  myOrientationsTree->initialize();
  myLoadsTree = new LoadsTree(myModelTree);
  myLoadsTree->initialize();
  myLoadsForcesTree = new LoadsForcesTree(myLoadsTree);
  myLoadsForcesTree->initialize();
  myLoadsPressuresTree = new LoadsPressuresTree(myLoadsTree);
  myLoadsPressuresTree->initialize();
  myLoadsHeatfluxesTree = new LoadsHeatfluxesTree(myLoadsTree);
  myLoadsHeatfluxesTree->initialize();
  myLoadsGravityTree = new LoadsGravityTree(myLoadsTree);
  myLoadsGravityTree->initialize();
  myLoadsCentrifugalTree = new LoadsCentrifugalTree(myLoadsTree);
  myLoadsCentrifugalTree->initialize();
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
  myHBCsTree = new HBCsTree(myModelTree);
  myHBCsTree->initialize();
  myHBCsDisplacementsTree = new HBCsDisplacementsTree(myHBCsTree);
  myHBCsDisplacementsTree->initialize();
  myHBCsTemperaturesTree = new HBCsTemperaturesTree(myHBCsTree);
  myHBCsTemperaturesTree->initialize();
  myStepsTree = new StepsTree(myModelTree);
  myStepsTree->initialize();
  myCustomLinesTree = new CustomLinesTree(myModelTree);
  myCustomLinesTree->initialize();
  myJobsTree = new JobsTree(myModelTree);
  myJobsTree->initialize();

  dock->setWidget(myModelTree);
      
  gui->add_dock_window(dock,"CalculiXComp",Qt::LeftDockWidgetArea,Qt::AllDockWidgetAreas);

  /*
  ViewMenu->add_dock_item(dock->toggleViewAction(),"CalculiXComp");
  std::cout << "dock labels 2 \n";
  ViewMenu->get_dock_labels(dock_labels);
  for (size_t i = 0; i < dock_labels.size(); i++)
  {
    std::cout << "---";
    std::cout << dock_labels[i].toStdString();
    std::cout << "\n";
  }
  */
  settings = new QSettings("CalculiXPlugin","CalculiXComp");
  dock->restoreGeometry(settings->value("geometry").toByteArray());
  gui->restoreState(settings->value("state").toByteArray());
    
  isInitialized = true;
}

void CCXDockWindowModelTree::clear()
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
    delete myOrientationsTree;
    delete myLoadsForcesTree;
    delete myLoadsPressuresTree;
    delete myLoadsHeatfluxesTree;
    delete myLoadsGravityTree;
    delete myLoadsCentrifugalTree;
    delete myLoadsTree;
    delete myBCsTemperaturesTree;
    delete myBCsDisplacementsTree;
    delete myBCsTree;
    delete myHistoryOutputsTree;
    delete myFieldOutputsTree;
    delete myInitialConditionsTree;
    delete myHBCsTemperaturesTree;
    delete myHBCsDisplacementsTree;
    delete myHBCsTree;
    delete myStepsTree;
    delete myCustomLinesTree;
    delete myJobsTree;
    delete myHighlight;
    delete myModelTree;
    delete dock;
    isInitialized = false;  
  }
  
}

void CCXDockWindowModelTree::update()
{
  settings->setValue("geometry",dock->saveGeometry());
  settings->setValue("state",gui->saveState());

  // update our dock items
  myModelTree->update();
  myBlocksTree->update(); 
  myNodesetTree->update(); 
  mySidesetTree->update(); 
  myMaterialTree->update(); 
  mySectionsTree->update();
  myConstraintsTree->update();
  mySurfaceInteractionsTree->update();
  myContactPairsTree->update();
  myAmplitudesTree->update();
  myOrientationsTree->update();
  //myLoadsTree->update();
  myLoadsForcesTree->update();
  myLoadsPressuresTree->update();
  myLoadsHeatfluxesTree->update();
  myLoadsGravityTree->update();
  myLoadsCentrifugalTree->update();
  //myBCsTree->update();
  myBCsDisplacementsTree->update();
  myBCsTemperaturesTree->update();
  myHistoryOutputsTree->update();
  myFieldOutputsTree->update();
  myInitialConditionsTree->update();
  //myHBCsTree->update();
  myHBCsDisplacementsTree->update();
  myHBCsTemperaturesTree->update();
  myStepsTree->update();
  myCustomLinesTree->update();
  myJobsTree->update();
}

void CCXDockWindowModelTree::reset()
{
  // reset, our trees, so clear all child items
  myModelTree->clear();
  myBlocksTree->clear(); 
  myNodesetTree->clear(); 
  mySidesetTree->clear(); 
  myMaterialTree->clear(); 
  mySectionsTree->clear();
  myConstraintsTree->clear();
  mySurfaceInteractionsTree->clear();
  myContactPairsTree->clear();
  myAmplitudesTree->clear();
  myOrientationsTree->clear();
  //myLoadsTree->clear();
  myLoadsForcesTree->clear();
  myLoadsPressuresTree->clear();
  myLoadsHeatfluxesTree->clear();
  myLoadsGravityTree->clear();
  myLoadsCentrifugalTree->clear();
  //myBCsTree->clear();
  myBCsDisplacementsTree->clear();
  myBCsTemperaturesTree->clear();
  myHistoryOutputsTree->clear();
  myFieldOutputsTree->clear();
  myInitialConditionsTree->clear();
  //myHBCsTree->clear();
  myHBCsDisplacementsTree->clear();
  myHBCsTemperaturesTree->clear();
  myStepsTree->clear();
  myCustomLinesTree->clear();
  myJobsTree->clear();
}