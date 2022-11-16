
#include "ModelTree.hpp"
#include "BlocksTree.hpp"
#include "CalculiXCoreInterface.hpp"


ModelTree::ModelTree(QDockWidget* parent):QTreeWidget(parent)
{
  CalculiXCoreInterface ccx_iface;
  
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
    ccx_iface.log_str("connected signals and slots");
  }    
}

ModelTree::~ModelTree()
{
}

void ModelTree::showContextMenu(const QPoint &pos)
{
  CalculiXCoreInterface ccx_iface;

  QTreeWidgetItem* item = this->itemAt(pos);

  BlocksTree* BlocksTreeItem;
  BlocksTreeItem = dynamic_cast<BlocksTree*>(item);
  if (BlocksTreeItem)
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      ccx_iface.log_str("right Click - ROOT ");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      ccx_iface.log_str("right Click - Block ID " + item->text(1).toStdString());
    }
  } 
}

void ModelTree::ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  CalculiXCoreInterface ccx_iface;

  BlocksTree* BlocksTreeItem;

  BlocksTreeItem = dynamic_cast<BlocksTree*>(item);
  if (BlocksTreeItem)
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      ccx_iface.log_str("double Click - ROOT ");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      ccx_iface.log_str("double Click - Block ID " + item->text(1).toStdString());
    }
  }
}
  
