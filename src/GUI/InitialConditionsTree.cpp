#include "InitialConditionsTree.hpp"
#include "CalculiXCoreInterface.hpp"

InitialConditionsTree::InitialConditionsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

InitialConditionsTree::~InitialConditionsTree()
{
}

void InitialConditionsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Initial Conditions");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("InitialConditionsTree")); 
  isInitialized = true;
}

void InitialConditionsTree::update()
{
  std::vector<std::vector<std::string>> initialconditions_tree_data;
  initialconditions_tree_data = ccx_iface->get_initialconditions_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < initialconditions_tree_data.size(); i++)
  {
    // check if initialcondition already exists as item
    
    ChildId = this->get_child_id(initialconditions_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addInitialCondition(QString::fromStdString(initialconditions_tree_data[i][0]),
        QString::fromStdString(initialconditions_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = initialconditions_tree_data[i][1];    

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
    temp_child = this->child(int(i)-1);

    for (size_t ii = 0; ii < initialconditions_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==initialconditions_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeInitialCondition(temp_child);
    }
  }
}

void InitialConditionsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void InitialConditionsTree::addInitialCondition(QString initialcondition_id, QString initialcondition_name)
{
  QTreeWidgetItem *InitialConditionTreeChild = new QTreeWidgetItem();
  
  InitialConditionTreeChild->setText(0, initialcondition_name);
    
  InitialConditionTreeChild->setText(1, initialcondition_id);

  InitialConditionTreeChild->setIcon(0,ccx_iface->getIcon2("InitialConditionsTree")); 

  this->addChild(InitialConditionTreeChild);
}

void InitialConditionsTree::removeInitialCondition(QTreeWidgetItem *initialcondition)
{
  this->removeChild(initialcondition);
  delete initialcondition;
}

int InitialConditionsTree::get_child_id(std::string initialcondition_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==initialcondition_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}