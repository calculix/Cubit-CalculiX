
#include "BlocksTree.hpp"
#include "CalculiXCoreInterface.hpp"

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
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("BlocksTree"));  
  isInitialized = true;
}

void BlocksTree::update()
{
  std::vector<std::vector<std::string>> blocks_tree_data;
  blocks_tree_data = ccx_iface->get_blocks_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < blocks_tree_data.size(); i++)
  {
    // check if block already exists as item
    
    ChildId = this->get_child_id(blocks_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addBlock(QString::fromStdString(blocks_tree_data[i][0]),
        QString::fromStdString(blocks_tree_data[i][1]),
        QString::fromStdString(blocks_tree_data[i][2]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      if (blocks_tree_data[i][2]!="")
      {
        str_check = blocks_tree_data[i][1] + " (" + blocks_tree_data[i][2] + ")";
      }else{
        str_check = blocks_tree_data[i][1] + " (---)";
      }    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }

  // check if item has been removed

  for (size_t i = this->childCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = this->child(i-1);

    for (size_t ii = 0; ii < blocks_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==blocks_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeBlock(temp_child);
    }
  }
}

void BlocksTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void BlocksTree::addBlock(QString block_id, QString block_name, QString element_type)
{
  QTreeWidgetItem *BlocksTreeChild = new QTreeWidgetItem();
  
  if (element_type!="")
  {
    BlocksTreeChild->setText(0, block_name + " (" + element_type + ")" );
  }else{
    BlocksTreeChild->setText(0, block_name + " (---)" );
  }
    
  BlocksTreeChild->setText(1, block_id);
  BlocksTreeChild->setIcon(0,ccx_iface->getIcon2("BlocksTree"));
  this->addChild(BlocksTreeChild);
}

void BlocksTree::removeBlock(QTreeWidgetItem *block)
{
  this->removeChild(block);
  delete block;
}

int BlocksTree::get_child_id(std::string block_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==block_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}