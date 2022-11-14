
#include "CCXDockWindow.hpp"
#include "Claro.hpp"
#include "ClaroViewMenu.hpp"
#include <iostream>
#include "BlocksTree.hpp"


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

  Claro* gui = Claro::instance();

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

  QDockWidget *dock = new QDockWidget(dock_title,gui);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  myModelTree = new QTreeWidget(dock);
  myModelTree->setColumnCount(2);
  myModelTree->setHeaderLabels(QStringList() << "Name" << "ID");
  myBlocksTree = new BlocksTree(myModelTree);
  myBlocksTree->initialize();
  
  QTreeWidgetItem *BlocksTreeChild;
  QTreeWidgetItem *BlocksTreeChild2;
  BlocksTreeChild = new QTreeWidgetItem();
  BlocksTreeChild->setText(0,"BlocksChild");
  BlocksTreeChild2 = new QTreeWidgetItem();
  BlocksTreeChild2->setText(0,"BlocksChild2");

  myBlocksTree->addChild(BlocksTreeChild);
  myBlocksTree->addChild(BlocksTreeChild2);
  
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
  
  isInitialized = true;
}

void CCXDockWindow::clear()
{
 // Remove all of our menu items.
  Claro* gui = Claro::instance();
  if(gui)
  {
    // Items are removed based on the component name
    gui->remove_dock_windows("CalculiXComp");

    isInitialized = false;  
  }
}
