#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "ConstraintsDeletePanel.hpp"
#include "ConstraintsModifyEquationPanel.hpp"
#include "ConstraintsModifyTiePanel.hpp"
#include "ConstraintsModifyRigidBodyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "ConstraintsTree.hpp"

#include "GUI/application/ModelTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include <QMenu>
ConstraintsTree::ConstraintsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

ConstraintsTree::~ConstraintsTree()
{
}

void ConstraintsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Constraints");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("ConstraintsTree"));
  isInitialized = true;
}

void ConstraintsTree::update()
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  std::vector<std::vector<std::string>> constraints_tree_data;
  constraints_tree_data = ccx_interactions_iface.get_constraints_tree_data();

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
    temp_child = this->child(int(i)-1);

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
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *ConstraintTreeChild = new QTreeWidgetItem();
  
  ConstraintTreeChild->setText(0, constraint_name);
    
  ConstraintTreeChild->setText(1, constraint_id);
  ConstraintTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Constraint));
  ConstraintTreeChild->setData(0, PropertyIdRole, constraint_id.toInt());

  ConstraintTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("ConstraintsTree"));

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
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==constraint_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void ConstraintsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Constraint", host, "CCXConstraintsCreateRigidBody");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Constraint", host, "CCXConstraintsCreateRigidBody");
  {
    auto* action = menu.addAction("Modify Constraint");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      ConstraintsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Constraint");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      ConstraintsTree::open_delete(selectedId, host);
    });
  }
}


void ConstraintsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  CalculiXInteractionsInterface core;
  ConstraintsSettings settings;
  std::string error;
  if (!core.get_constraint_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot open Constraint %d: %s\n", id, error.c_str());
    return;
  }

  QString marker;
  switch (settings.type)
  {
    case 1: marker = "CCXConstraintsModifyRigidBody"; break;
    case 2: marker = "CCXConstraintsModifyTie"; break;
    case 3: marker = "CCXConstraintsModifyEquation"; break;
    default:
      PRINT_ERROR("This stored type has no modify panel.\n");
      return;
  }
  host.setWidgetInCmdPanelMarker(marker);

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<ConstraintsModifyRigidBodyPanel*>(widget))
    {
      if (settings.type == 1)
      {
        panel->setConstraint(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<ConstraintsModifyTiePanel*>(widget))
    {
      if (settings.type == 2)
      {
        panel->setConstraint(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<ConstraintsModifyEquationPanel*>(widget))
    {
      if (settings.type == 3)
      {
        panel->setConstraint(id);
        return;
      }
    }
  }
  PRINT_ERROR("Unable to open the Constraint modify panel.\n");
}

void ConstraintsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXConstraintsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<ConstraintsDeletePanel*>(widget))
    {
      panel->setConstraint(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Constraint delete panel.\n");
}

void ConstraintsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXConstraintsCreate");
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
