
#include "ModelTree.hpp"
#include "BlocksTree.hpp"
#include "CalculiXCoreInterface.hpp"

#include "Claro.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"

#include "MyCmdWidgetFactory.hpp"
#include "CommandButtonPanel.hpp"


ModelTree::ModelTree(QDockWidget* parent):QTreeWidget(parent)
{
  ccx_iface = new CalculiXCoreInterface();

  gui = Claro::instance();

  nav_model = gui->navigation_model();
  if (nav_model)
  {
    ccx_iface->log_str("NavigationModel access");
  }
  
  cmdpanel = dynamic_cast<CommandButtonPanel*>(gui->command_buttons());
  if (nav_model)
  {
    ccx_iface->log_str("CommandButtonPanel access");
  }

  this->setColumnCount(2);
  this->setHeaderLabels(QStringList() << "Name" << "ID");
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  bool success;
  success = false;

  success = connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(showContextMenu(const QPoint &)));

  success = connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
          this, SLOT(ModelTreeItemDoubleClicked(QTreeWidgetItem*, int)));
  
  if (success)
  {
    ccx_iface->log_str("connected signals and slots");
  }    

  //to get our current right clicked action
  // contextMenuAction[0][0] => Tree
  // contextMenuAction[0][1] => action
  // contextMenuAction[0][2] => ID
  std::vector<int> v = {-1,-1,-1};
  contextMenuAction.push_back(v);
}

ModelTree::~ModelTree()
{
}

void ModelTree::showContextMenu(const QPoint &pos)
{
  QTreeWidgetItem* item = this->itemAt(pos);
  if (item)
  {   
    BlocksTree* BlocksTreeItem;
    BlocksTreeItem = dynamic_cast<BlocksTree*>(item);
    if (BlocksTreeItem)
    {
      if (BlocksTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Assign CCX Element Type",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(execContextMenuAction()));
        contextMenu.addAction(&action1);      
        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 0;
        contextMenuAction[0][1] = 0;
      }
      } else {
      if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
      {
        //ccx_iface->log_str("right Click - Block ID " + item->text(1).toStdString());
        QMenu contextMenu("Context Menu",this);
        QAction action1("Assign CCX Element Type",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(execContextMenuAction()));
        contextMenu.addAction(&action1);   
        contextMenu.exec(mapToGlobal(pos));   

        contextMenuAction[0][0] = 0;
        contextMenuAction[0][1] = 0;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
      }
    }
  }
}

void ModelTree::ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  BlocksTree* BlocksTreeItem;

  BlocksTreeItem = dynamic_cast<BlocksTree*>(item);
  if (BlocksTreeItem)
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      //ccx_iface->log_str("double Click - ROOT ");
      this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
    }
  }
}
  
void ModelTree::setWidgetInCmdPanelMarker(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  NavigationNode* node = nav_model->getMarkedNode(name);
  widget_index = nav_model->getIndex(node);
  cmdpanel->setCurrent(widget_index);
}

void ModelTree::setWidgetInCmdPanelPath(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  NavigationNode* node = nav_model->getNode(name);
  widget_index = nav_model->getIndex(node);
  cmdpanel->setCurrent(widget_index);
}

void ModelTree::execContextMenuAction(){
  if (contextMenuAction[0][0]!=-1 && contextMenuAction[0][1]!=-1)
  {
    if (contextMenuAction[0][0]==0) //Blocktree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
      }
    }
  }
  contextMenuAction[0][0]=-1;
  contextMenuAction[0][1]=-1;
  contextMenuAction[0][2]=-1;
}