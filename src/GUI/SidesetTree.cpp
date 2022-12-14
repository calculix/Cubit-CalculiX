
#include "SidesetTree.hpp"
#include "CalculiXCoreInterface.hpp"

SidesetTree::SidesetTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

SidesetTree::~SidesetTree()
{
}

void SidesetTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Side Sets");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void SidesetTree::update()
{
  std::vector<std::vector<std::string>> sideset_tree_data;
  sideset_tree_data = ccx_iface->get_sideset_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < sideset_tree_data.size(); i++)
  {
    // check if sideset already exists as item
    
    ChildId = this->get_child_id(sideset_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addSideset(QString::fromStdString(sideset_tree_data[i][0]),
        QString::fromStdString(sideset_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = sideset_tree_data[i][1];    

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

    for (size_t ii = 0; ii < sideset_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==sideset_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeSideset(temp_child);
    }
  }
}

void SidesetTree::clear()
{
 // Remove all of our tree items.
 // not implemented yet
}

void SidesetTree::addSideset(QString sideset_id, QString sideset_name)
{
  QTreeWidgetItem *SidesetTreeChild = new QTreeWidgetItem();
  
  SidesetTreeChild->setText(0, sideset_name);
    
  SidesetTreeChild->setText(1, sideset_id);

  this->addChild(SidesetTreeChild);
}

void SidesetTree::removeSideset(QTreeWidgetItem *sideset)
{
  this->removeChild(sideset);
  delete sideset;
}

int SidesetTree::get_child_id(std::string sideset_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==sideset_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}