#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "LoadsTrajectoryDeletePanel.hpp"
#include "LoadsTrajectoryBodyHeatfluxSphereModifyPanel.hpp"
#include "LoadsTrajectoryHeatfluxModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "LoadsTrajectoryTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
LoadsTrajectoryTree::LoadsTrajectoryTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

LoadsTrajectoryTree::~LoadsTrajectoryTree()
{
}

void LoadsTrajectoryTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Trajectory");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsTrajectoryTree"));
  isInitialized = true;
}

void LoadsTrajectoryTree::update()
{
  CalculiXLoadsInterface ccx_loads_iface;

  std::vector<std::vector<std::string>> loads_tree_data;
  loads_tree_data = ccx_loads_iface.get_loadstrajectory_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < loads_tree_data.size(); i++)
  {
    // check if load already exists as item
    
    ChildId = this->get_child_id(loads_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addLoad(QString::fromStdString(loads_tree_data[i][0]),
        QString::fromStdString(loads_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = loads_tree_data[i][1];    

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

    for (size_t ii = 0; ii < loads_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==loads_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeLoad(temp_child);
    }
  }
}

void LoadsTrajectoryTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void LoadsTrajectoryTree::addLoad(QString load_id, QString load_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *LoadTreeChild = new QTreeWidgetItem();
  
  LoadTreeChild->setText(0, load_name);
    
  LoadTreeChild->setText(1, load_id);
  LoadTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Trajectory));
  LoadTreeChild->setData(0, PropertyIdRole, load_id.toInt());

  LoadTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsTrajectoryTree"));

  this->addChild(LoadTreeChild);
}

void LoadsTrajectoryTree::removeLoad(QTreeWidgetItem *load)
{
  this->removeChild(load);
  delete load;
}

int LoadsTrajectoryTree::get_child_id(std::string load_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==load_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void LoadsTrajectoryTree::populateContextMenu(
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
    addPanelAction(menu, "Create Trajectory", host, "CCXLoadsTrajectory");
    addCommandAction(menu, "Draw Trajectories", QString("ccx draw load_trajectory_all"));
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

  addPanelAction(menu, "Create Trajectory", host, "CCXLoadsTrajectory");
  {
    auto* action = menu.addAction("Modify Trajectory");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      LoadsTrajectoryTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Trajectory");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      LoadsTrajectoryTree::open_delete(selectedId, host);
    });
  }
  addCommandAction(menu, "Draw Trajectory", QString("ccx draw load trajectory ") + QString::number(entityId));
}


void LoadsTrajectoryTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  CalculiXLoadsInterface core;
  LoadsTrajectorySettings settings;
  std::string error;
  if (!core.get_trajectory_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot open Trajectory %d: %s\n", id, error.c_str());
    return;
  }

  QString marker;
  switch (settings.type)
  {
    case 1: marker = "CCXLoadsTrajectoryHeatfluxModify"; break;
    case 2: marker = "CCXLoadsTrajectoryBodyHeatfluxSphereModify"; break;
    default:
      PRINT_ERROR("This stored type has no modify panel.\n");
      return;
  }
  host.setWidgetInCmdPanelMarker(marker);

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<LoadsTrajectoryHeatfluxModifyPanel*>(widget))
    {
      if (settings.type == 1)
      {
        panel->setTrajectory(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<LoadsTrajectoryBodyHeatfluxSphereModifyPanel*>(widget))
    {
      if (settings.type == 2)
      {
        panel->setTrajectory(id);
        return;
      }
    }
  }
  PRINT_ERROR("Unable to open the Trajectory modify panel.\n");
}

void LoadsTrajectoryTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXLoadsTrajectoryDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<LoadsTrajectoryDeletePanel*>(widget))
    {
      panel->setTrajectory(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Trajectory delete panel.\n");
}

void LoadsTrajectoryTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXLoadsTrajectoryHeatfluxCreate");
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
