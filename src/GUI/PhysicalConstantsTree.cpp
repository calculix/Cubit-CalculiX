#include "PhysicalConstantsTree.hpp"
#include "CalculiXCoreInterface.hpp"

PhysicalConstantsTree::PhysicalConstantsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

PhysicalConstantsTree::~PhysicalConstantsTree()
{
}

void PhysicalConstantsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Physical Constants");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("PhysicalConstantsTree")); 
  isInitialized = true;
}

void PhysicalConstantsTree::update()
{
  std::vector<std::vector<std::string>> physicalconstants_tree_data;
  physicalconstants_tree_data = ccx_iface->get_physicalconstants_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < physicalconstants_tree_data.size(); i++)
  {
    // check if PhysicalConstants already exists as item
    
    ChildId = this->get_child_id(physicalconstants_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addPhysicalConstants(QString::fromStdString(physicalconstants_tree_data[i][0]),
        QString::fromStdString(physicalconstants_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = physicalconstants_tree_data[i][1];    

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

    for (size_t ii = 0; ii < physicalconstants_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==physicalconstants_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removePhysicalConstants(temp_child);
    }
  }
}

void PhysicalConstantsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void PhysicalConstantsTree::addPhysicalConstants(QString PhysicalConstants_id, QString name)
{
  QTreeWidgetItem *PhysicalConstantsTreeChild = new QTreeWidgetItem();
  
  PhysicalConstantsTreeChild->setText(0, name);
    
  PhysicalConstantsTreeChild->setText(1, PhysicalConstants_id);

  PhysicalConstantsTreeChild->setIcon(0,ccx_iface->getIcon2("PhysicalConstantsTree"));

  this->addChild(PhysicalConstantsTreeChild);
}

void PhysicalConstantsTree::removePhysicalConstants(QTreeWidgetItem *PhysicalConstants)
{
  this->removeChild(PhysicalConstants);
  delete PhysicalConstants;
}

int PhysicalConstantsTree::get_child_id(std::string PhysicalConstants_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==PhysicalConstants_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}