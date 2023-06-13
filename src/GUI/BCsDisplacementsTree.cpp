#include "BCsDisplacementsTree.hpp"
#include "CalculiXCoreInterface.hpp"

BCsDisplacementsTree::BCsDisplacementsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BCsDisplacementsTree::~BCsDisplacementsTree()
{
}

void BCsDisplacementsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Displacements");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("BCsDisplacementsTree")); 
  isInitialized = true;
}

void BCsDisplacementsTree::update()
{
  std::vector<std::vector<std::string>> bcs_tree_data;
  bcs_tree_data = ccx_iface->get_bcsdisplacements_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < bcs_tree_data.size(); i++)
  {
    // check if bc already exists as item
    
    ChildId = this->get_child_id(bcs_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addBC(QString::fromStdString(bcs_tree_data[i][0]),
        QString::fromStdString(bcs_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = bcs_tree_data[i][1];    

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

    for (size_t ii = 0; ii < bcs_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==bcs_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeBC(temp_child);
    }
  }
}

void BCsDisplacementsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void BCsDisplacementsTree::addBC(QString bc_id, QString bc_name)
{
  QTreeWidgetItem *BCTreeChild = new QTreeWidgetItem();
  
  BCTreeChild->setText(0, bc_name);
    
  BCTreeChild->setText(1, bc_id);
  
  BCTreeChild->setIcon(0,ccx_iface->getIcon2("BCsDisplacementsTree"));

  this->addChild(BCTreeChild);
}

void BCsDisplacementsTree::removeBC(QTreeWidgetItem *bc)
{
  this->removeChild(bc);
  delete bc;
}

int BCsDisplacementsTree::get_child_id(std::string bc_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==bc_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}