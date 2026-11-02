#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "InitialConditionsDeletePanel.hpp"
#include "InitialConditionsModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "InitialConditionsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
InitialConditionsTree::InitialConditionsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

InitialConditionsTree::~InitialConditionsTree()
{
}

void InitialConditionsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Initial Conditions");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("InitialConditionsTree"));
  isInitialized = true;
}

void InitialConditionsTree::update()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::vector<std::string>> initialconditions_tree_data;
  initialconditions_tree_data = ccx_steps_iface.get_initialconditions_tree_data();

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
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *InitialConditionTreeChild = new QTreeWidgetItem();
  
  InitialConditionTreeChild->setText(0, initialcondition_name);
    
  InitialConditionTreeChild->setText(1, initialcondition_id);
  InitialConditionTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::InitialCondition));
  InitialConditionTreeChild->setData(0, PropertyIdRole, initialcondition_id.toInt());

  InitialConditionTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("InitialConditionsTree"));

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

void InitialConditionsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Initial Condition", host, "CCXInitialConditionsCreate");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Initial Condition", host, "CCXInitialConditionsCreate");
  {
    auto* action = menu.addAction("Modify Initial Condition");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      InitialConditionsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Initial Condition");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      InitialConditionsTree::open_delete(selectedId, host);
    });
  }
}


void InitialConditionsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXInitialConditionsModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<InitialConditionsModifyPanel*>(widget))
    {
      panel->setInitialCondition(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the InitialCondition modify panel.\n");
}

void InitialConditionsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXInitialConditionsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<InitialConditionsDeletePanel*>(widget))
    {
      panel->setInitialCondition(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the InitialCondition delete panel.\n");
}

void InitialConditionsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXInitialConditionsCreate");
    return;
  }

  if (item->parent() != this)
  {
    return;
  }

  int id = -1;
  if (!readItemId(item, id))
  {
    return;
  }
  open_modify(id, host);
}
