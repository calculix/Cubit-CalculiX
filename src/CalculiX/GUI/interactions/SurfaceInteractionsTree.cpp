#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "SurfaceInteractionsDeletePanel.hpp"
#include "SurfaceInteractionsModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "SurfaceInteractionsTree.hpp"

#include "GUI/application/ModelTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include <QMenu>
SurfaceInteractionsTree::SurfaceInteractionsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

SurfaceInteractionsTree::~SurfaceInteractionsTree()
{
}

void SurfaceInteractionsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Surface Interactions");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("SurfaceInteractionsTree"));
  isInitialized = true;
}

void SurfaceInteractionsTree::update()
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  std::vector<std::vector<std::string>> surfaceinteractions_tree_data;
  surfaceinteractions_tree_data = ccx_interactions_iface.get_surfaceinteractions_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < surfaceinteractions_tree_data.size(); i++)
  {
    // check if surfaceinteraction already exists as item
    
    ChildId = this->get_child_id(surfaceinteractions_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addSurfaceInteraction(QString::fromStdString(surfaceinteractions_tree_data[i][0]),
        QString::fromStdString(surfaceinteractions_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = surfaceinteractions_tree_data[i][1];    

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

    for (size_t ii = 0; ii < surfaceinteractions_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==surfaceinteractions_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeSurfaceInteraction(temp_child);
    }
  }
}

void SurfaceInteractionsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void SurfaceInteractionsTree::addSurfaceInteraction(QString surfaceinteraction_id, QString surfaceinteraction_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *SurfaceInteractionTreeChild = new QTreeWidgetItem();
  
  SurfaceInteractionTreeChild->setText(0, surfaceinteraction_name);
    
  SurfaceInteractionTreeChild->setText(1, surfaceinteraction_id);
  SurfaceInteractionTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::SurfaceInteraction));
  SurfaceInteractionTreeChild->setData(0, PropertyIdRole, surfaceinteraction_id.toInt());

  SurfaceInteractionTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("SurfaceInteractionsTree"));

  this->addChild(SurfaceInteractionTreeChild);
}

void SurfaceInteractionsTree::removeSurfaceInteraction(QTreeWidgetItem *surfaceinteraction)
{
  this->removeChild(surfaceinteraction);
  delete surfaceinteraction;
}

int SurfaceInteractionsTree::get_child_id(std::string surfaceinteraction_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==surfaceinteraction_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void SurfaceInteractionsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Surface Interaction", host, "CCXSurfaceInteractionsCreate");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Surface Interaction", host, "CCXSurfaceInteractionsCreate");
  {
    auto* action = menu.addAction("Modify Surface Interaction");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      SurfaceInteractionsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Surface Interaction");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      SurfaceInteractionsTree::open_delete(selectedId, host);
    });
  }
}


void SurfaceInteractionsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXSurfaceInteractionsModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<SurfaceInteractionsModifyPanel*>(widget))
    {
      panel->setSurfaceInteraction(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the SurfaceInteraction modify panel.\n");
}

void SurfaceInteractionsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXSurfaceInteractionsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<SurfaceInteractionsDeletePanel*>(widget))
    {
      panel->setSurfaceInteraction(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the SurfaceInteraction delete panel.\n");
}

void SurfaceInteractionsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXSurfaceInteractionsCreate");
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
