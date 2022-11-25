
#include "NodesetTree.hpp"
#include "CalculiXCoreInterface.hpp"

NodesetTree::NodesetTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

NodesetTree::~NodesetTree()
{
}

void NodesetTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Node Sets");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void NodesetTree::update()
{
  std::vector<std::vector<std::string>> nodeset_tree_data;
  nodeset_tree_data = ccx_iface->get_nodeset_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < nodeset_tree_data.size(); i++)
  {
    // check if block already exists as item
    
    ChildId = this->get_child_id(nodeset_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addNodeset(QString::fromStdString(nodeset_tree_data[i][0]),
        QString::fromStdString(nodeset_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = nodeset_tree_data[i][1];    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }

  // check if item has been removed

  for (size_t i = 0; i < this->childCount(); i++)
  { 
    erase_item = true;
    temp_child = this->child(i);

    for (size_t ii = 0; ii < nodeset_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==nodeset_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeNodeset(temp_child);
    }
  }
}

void NodesetTree::clear()
{
 // Remove all of our tree items.
 // not implemented yet
}

void NodesetTree::addNodeset(QString nodeset_id, QString nodeset_name)
{
  QTreeWidgetItem *NodesetTreeChild = new QTreeWidgetItem();
  
  NodesetTreeChild->setText(0, nodeset_name);
    
  NodesetTreeChild->setText(1, nodeset_id);

  this->addChild(NodesetTreeChild);
}

void NodesetTree::removeNodeset(QTreeWidgetItem *nodeset)
{
  this->removeChild(nodeset);
  delete nodeset;
}

int NodesetTree::get_child_id(std::string nodeset_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==nodeset_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}