#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "DampingDeletePanel.hpp"
#include "DampingModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
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
  setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Damping));
  setData(0, PropertyIdRole, 0);
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
  dampingTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Damping));
  dampingTreeChild->setData(0, PropertyIdRole, damping_id.toInt());

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

  int selectedId = 0;
  if (clickedItem != this && !readItemId(clickedItem, selectedId))
  {
    return;
  }

  if (clickedItem == this)
  {
    {
      auto* action = menu.addAction("Modify Damping");
      QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
      {
        DampingTree::open_modify(0, host);
      });
    }
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  {
    auto* action = menu.addAction("Modify Damping");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      DampingTree::open_modify(0, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Damping");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      DampingTree::open_delete(selectedId, host);
    });
  }
}


void DampingTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXDampingModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<DampingModifyPanel*>(widget))
    {
      panel->setDamping(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Damping modify panel.\n");
}

void DampingTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXDampingDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<DampingDeletePanel*>(widget))
    {
      panel->selectCoefficient(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Damping delete panel.\n");
}

void DampingTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  open_modify(0, host);
}
