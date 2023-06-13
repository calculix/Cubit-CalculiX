
#include "ConstraintsTree.hpp"
#include "CalculiXCoreInterface.hpp"

ConstraintsTree::ConstraintsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

ConstraintsTree::~ConstraintsTree()
{
}

void ConstraintsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Constraints");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("ConstraintsTree"));
  isInitialized = true;
}

void ConstraintsTree::update()
{
  std::vector<std::vector<std::string>> constraints_tree_data;
  constraints_tree_data = ccx_iface->get_constraints_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < constraints_tree_data.size(); i++)
  {
    // check if constraint already exists as item
    
    ChildId = this->get_child_id(constraints_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addConstraint(QString::fromStdString(constraints_tree_data[i][0]),
        QString::fromStdString(constraints_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = constraints_tree_data[i][1];    

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

    for (size_t ii = 0; ii < constraints_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==constraints_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeConstraint(temp_child);
    }
  }
}

void ConstraintsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void ConstraintsTree::addConstraint(QString constraint_id, QString constraint_name)
{
  QTreeWidgetItem *ConstraintTreeChild = new QTreeWidgetItem();
  
  ConstraintTreeChild->setText(0, constraint_name);
    
  ConstraintTreeChild->setText(1, constraint_id);

  ConstraintTreeChild->setIcon(0,ccx_iface->getIcon2("ConstraintsTree"));

  this->addChild(ConstraintTreeChild);
}

void ConstraintsTree::removeConstraint(QTreeWidgetItem *constraint)
{
  this->removeChild(constraint);
  delete constraint;
}

int ConstraintsTree::get_child_id(std::string constraint_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==constraint_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}