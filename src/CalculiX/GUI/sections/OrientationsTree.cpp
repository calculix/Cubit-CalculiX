#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "OrientationsDeletePanel.hpp"
#include "OrientationsModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "OrientationsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
OrientationsTree::OrientationsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

OrientationsTree::~OrientationsTree()
{
}

void OrientationsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Orientations");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("OrientationsTree"));
  isInitialized = true;
}

void OrientationsTree::update()
{
  CalculiXSectionsInterface ccx_sections_iface;

  std::vector<std::vector<std::string>> orientations_tree_data;
  orientations_tree_data = ccx_sections_iface.get_orientations_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < orientations_tree_data.size(); i++)
  {
    // check if orientation already exists as item
    
    ChildId = this->get_child_id(orientations_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addOrientation(QString::fromStdString(orientations_tree_data[i][0]),
        QString::fromStdString(orientations_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = orientations_tree_data[i][1];    

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

    for (size_t ii = 0; ii < orientations_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==orientations_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeOrientation(temp_child);
    }
  }
}

void OrientationsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void OrientationsTree::addOrientation(QString orientation_id, QString name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *OrientationTreeChild = new QTreeWidgetItem();
  
  OrientationTreeChild->setText(0, name);
    
  OrientationTreeChild->setText(1, orientation_id);
  OrientationTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Orientation));
  OrientationTreeChild->setData(0, PropertyIdRole, orientation_id.toInt());

  OrientationTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("OrientationsTree"));

  this->addChild(OrientationTreeChild);
}

void OrientationsTree::removeOrientation(QTreeWidgetItem *orientation)
{
  this->removeChild(orientation);
  delete orientation;
}

int OrientationsTree::get_child_id(std::string orientation_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==orientation_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void OrientationsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Orientation", host, "CCXOrientationsCreate");
    addCommandAction(menu, "Draw Orientations", QString("ccx draw orientation_all"));
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Orientation", host, "CCXOrientationsCreate");
  {
    auto* action = menu.addAction("Modify Orientation");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      OrientationsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Orientation");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      OrientationsTree::open_delete(selectedId, host);
    });
  }
}


void OrientationsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXOrientationsModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<OrientationsModifyPanel*>(widget))
    {
      panel->setOrientation(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Orientation modify panel.\n");
}

void OrientationsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXOrientationsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<OrientationsDeletePanel*>(widget))
    {
      panel->setOrientation(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Orientation delete panel.\n");
}

void OrientationsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXOrientationsCreate");
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
