#include "DampingTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
DampingTree::DampingTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

DampingTree::~DampingTree()
{
}

void DampingTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Damping");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("DampingTree"));
  isInitialized = true;
}

void DampingTree::update()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::vector<std::string>> damping_tree_data;
  damping_tree_data = ccx_steps_iface.get_damping_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < damping_tree_data.size(); i++)
  {
    // check if damping already exists as item
    
    ChildId = this->get_child_id(damping_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addDamping(QString::fromStdString(damping_tree_data[i][0]),
        QString::fromStdString(damping_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = damping_tree_data[i][1];    

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

    for (size_t ii = 0; ii < damping_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==damping_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeDamping(temp_child);
    }
  }
}

void DampingTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void DampingTree::addDamping(QString damping_id, QString name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *dampingTreeChild = new QTreeWidgetItem();
  
  dampingTreeChild->setText(0, name);
    
  dampingTreeChild->setText(1, damping_id);

  dampingTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("DampingTree"));

  this->addChild(dampingTreeChild);
}

void DampingTree::removeDamping(QTreeWidgetItem *damping)
{
  this->removeChild(damping);
  delete damping;
}

int DampingTree::get_child_id(std::string damping_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==damping_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void DampingTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    addPanelAction(menu, "Modify Damping", host, "CCXDampingModify");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Modify Damping", host, "CCXDampingModify");
  addPanelAction(menu, "Delete Damping", host, "CCXDampingDelete");
}
