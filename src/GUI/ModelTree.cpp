
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
}

ModelTree::~ModelTree()
{
}

void ModelTree::showContextMenu(const QPoint &pos)
{
  QTreeWidgetItem* item = this->itemAt(pos);

  BlocksTree* BlocksTreeItem;
  BlocksTreeItem = dynamic_cast<BlocksTree*>(item);
  if (BlocksTreeItem)
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      ccx_iface->log_str("right Click - ROOT ");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      ccx_iface->log_str("right Click - Block ID " + item->text(1).toStdString());
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
      this->setWidgetInCmdPanel("MySecondLevelNode1");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      ccx_iface->log_str("double Click - Block ID " + item->text(1).toStdString());
      this->setWidgetInCmdPanel("MySecondLevelNode2");
    }
  }
}
  
void ModelTree::setWidgetInCmdPanel(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  NavigationNode* node = nav_model->getMarkedNode(name);
  widget_index = nav_model->getIndex(node);
  cmdpanel->setCurrent(widget_index);
}

void ModelTree::getWidgetInCmdPanel() // get Widget from navigation model
{
  QModelIndex widget_index;
  widget_index = cmdpanel->getCurrent();
  NavigationNode* node = nav_model->getNode(widget_index);
  ccx_iface->log_str(node->getMarker().toStdString());
}