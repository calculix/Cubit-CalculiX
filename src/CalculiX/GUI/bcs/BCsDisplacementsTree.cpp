#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "BCsDisplacementsDeletePanel.hpp"
#include "BCsDisplacementsModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "BCsDisplacementsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
BCsDisplacementsTree::BCsDisplacementsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BCsDisplacementsTree::~BCsDisplacementsTree()
{
}

void BCsDisplacementsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Displacements");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("BCsDisplacementsTree"));
  isInitialized = true;
}

void BCsDisplacementsTree::update()
{
  CalculiXBCsInterface ccx_bcs_iface;

  std::vector<std::vector<std::string>> bcs_tree_data;
  bcs_tree_data = ccx_bcs_iface.get_bcsdisplacements_tree_data();

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
    temp_child = this->child(int(i)-1);

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
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *BCTreeChild = new QTreeWidgetItem();
  
  BCTreeChild->setText(0, bc_name);
    
  BCTreeChild->setText(1, bc_id);
  BCTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Displacement));
  BCTreeChild->setData(0, PropertyIdRole, bc_id.toInt());
  
  BCTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("BCsDisplacementsTree"));

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
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==bc_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void BCsDisplacementsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Displacement", host, "CCXBCsDisplacementsCreate");
    addCommandAction(menu, "Draw Displacements", QString("ccx draw bc_displacement_all"));
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  int entityId = -1;
  if (!readItemId(clickedItem, entityId))
  {
    return;
  }

  addPanelAction(menu, "Create Displacement", host, "CCXBCsDisplacementsCreate");
  {
    auto* action = menu.addAction("Modify Displacement");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      BCsDisplacementsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Displacement");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      BCsDisplacementsTree::open_delete(selectedId, host);
    });
  }
  addCommandAction(menu, "Draw Displacement", QString("ccx draw bc displacement ") + QString::number(entityId));
}


void BCsDisplacementsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXBCsDisplacementsModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<BCsDisplacementsModifyPanel*>(widget))
    {
      panel->setDisplacement(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Displacement modify panel.\n");
}

void BCsDisplacementsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXBCsDisplacementsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<BCsDisplacementsDeletePanel*>(widget))
    {
      panel->setDisplacement(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Displacement delete panel.\n");
}

void BCsDisplacementsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXBCsDisplacementsCreate");
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
