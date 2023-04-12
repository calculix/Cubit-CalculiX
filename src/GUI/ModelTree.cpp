
#include "ModelTree.hpp"
#include "BlocksTree.hpp"
#include "NodesetTree.hpp"
#include "SidesetTree.hpp"
#include "MaterialTree.hpp"
#include "SectionsTree.hpp"
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
  /*
  if (nav_model)
  {
    ccx_iface->log_str("NavigationModel access");
  }
  */

  cmdpanel = dynamic_cast<CommandButtonPanel*>(gui->command_buttons());
  /*
  if (nav_model)
  {
    ccx_iface->log_str("CommandButtonPanel access");
  }
  */

  this->setColumnCount(2);
  this->setHeaderLabels(QStringList() << "Name" << "ID");
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  bool success;
  success = false;

  success = connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(showContextMenu(const QPoint &)));

  success = connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
          this, SLOT(ModelTreeItemDoubleClicked(QTreeWidgetItem*, int)));
  
  /*if (success)
  {
    ccx_iface->log_str("connected signals and slots");
  }*/   

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
    NodesetTree* NodesetTreeItem;
    SidesetTree* SidesetTreeItem;
    MaterialTree* MaterialTreeItem;
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item))
    {
      if (BlocksTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Assign CCX Element Type",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);      
        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 0;
      }
    }else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item))
    {
      if (NodesetTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Nodeset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);      
        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 1;
      }
    }else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item))
    {
      if (SidesetTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Sideset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);      
        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 2;
      }
    }else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item))
    {
      if (MaterialTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Open Material Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);      
        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 3;
      }
    }else 
    {
      if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
      {
        //ccx_iface->log_str("right Click - Block ID " + item->text(1).toStdString());
        QMenu contextMenu("Context Menu",this);
        QAction action1("Assign CCX Element Type",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);   
        contextMenu.exec(mapToGlobal(pos));   

        contextMenuAction[0][0] = 0;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
      } else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Nodeset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Nodeset",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Nodeset",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 1;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
      } else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Sideset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Sideset",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Sideset",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 2;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
      } else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Open Material Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenu.exec(mapToGlobal(pos));

        contextMenuAction[0][0] = 3;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
      }
    }
  }
}

void ModelTree::ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  BlocksTree* BlocksTreeItem;
  NodesetTree* NodesetTreeItem;
  SidesetTree* SidesetTreeItem;
  MaterialTree* MaterialTreeItem;

  if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item))
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
    }
  }else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item))
  {
    if (NodesetTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("ExodusCreateNodeset");
    }
  }else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item))
  {
    if (SidesetTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("ExodusCreateSideset");
    }
  }else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item))
  {
    if (MaterialTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("ExodusCreateSideset");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
    } else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("ExodusRemoveContentsNodeset");
    } else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("ExodusRemoveContentsSideset");
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
        this->setWidgetInCmdPanelMarker("CCXBlocksElementType");
      }
    }else if (contextMenuAction[0][0]==1) //NodesetTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("ExodusCreateNodeset");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("ExodusRemoveContentsNodeset");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("ExodusDeleteNodeset");
      }  
    }else if (contextMenuAction[0][0]==2) //SidesetTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("ExodusCreateSideset");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("ExodusRemoveContentsSideset");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("ExodusDeleteSideset");
      }  
    }else if (contextMenuAction[0][0]==3) //MaterialTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("ExodusCreateSideset");
      }  
    }
  }
  contextMenuAction[0][0]=-1;
  contextMenuAction[0][1]=-1;
  contextMenuAction[0][2]=-1;
}

void ModelTree::ContextMenuAction1(){
  contextMenuAction[0][1]=0;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction2(){
  contextMenuAction[0][1]=1;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction3(){
  contextMenuAction[0][1]=2;
  this->execContextMenuAction();
}