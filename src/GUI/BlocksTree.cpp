
#include "BlocksTree.hpp"

//BlocksTree::BlocksTree(QTreeWidget* parent, int type) :
BlocksTree::BlocksTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BlocksTree::~BlocksTree()
{
}

void BlocksTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Element Sets");
  /*
  QTreeWidgetItem *BlocksTreeChild;
  QTreeWidgetItem *BlocksTreeChild2;
  BlocksTreeChild = new QTreeWidgetItem();
  BlocksTreeChild->setText(0,"BlocksChild");
  BlocksTreeChild2 = new QTreeWidgetItem();
  BlocksTreeChild2->setText(0,"BlocksChild2");

  BlocksTree->addChild(BlocksTreeChild);
  BlocksTree->addChild(BlocksTreeChild2);
  */

  isInitialized = true;
}

void BlocksTree::clear()
{
 // Remove all of our tree items.
  
}
