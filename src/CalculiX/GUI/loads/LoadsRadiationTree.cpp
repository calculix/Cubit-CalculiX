#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "LoadsRadiationDeletePanel.hpp"
#include "LoadsRadiationModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "LoadsRadiationTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
LoadsRadiationTree::LoadsRadiationTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

LoadsRadiationTree::~LoadsRadiationTree()
{
}

void LoadsRadiationTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Radiation");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsRadiationTree"));
  isInitialized = true;
}

void LoadsRadiationTree::update()
{
  CalculiXLoadsInterface ccx_loads_iface;

  std::vector<std::vector<std::string>> loads_tree_data;
  loads_tree_data = ccx_loads_iface.get_loadsradiation_tree_data();

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

void LoadsRadiationTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void LoadsRadiationTree::addLoad(QString load_id, QString load_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *LoadTreeChild = new QTreeWidgetItem();
  
  LoadTreeChild->setText(0, load_name);
    
  LoadTreeChild->setText(1, load_id);
  LoadTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Radiation));
  LoadTreeChild->setData(0, PropertyIdRole, load_id.toInt());

  LoadTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsRadiationTree"));

  this->addChild(LoadTreeChild);
}

void LoadsRadiationTree::removeLoad(QTreeWidgetItem *load)
{
  this->removeChild(load);
  delete load;
}

int LoadsRadiationTree::get_child_id(std::string load_id)
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

void LoadsRadiationTree::populateContextMenu(
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
    addPanelAction(menu, "Create Radiation", host, "CCXLoadsRadiationCreate");
    addCommandAction(menu, "Draw Radiations", QString("ccx draw load_radiation_all"));
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

  addPanelAction(menu, "Create Radiation", host, "CCXLoadsRadiationCreate");
  {
    auto* action = menu.addAction("Modify Radiation");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      LoadsRadiationTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Radiation");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      LoadsRadiationTree::open_delete(selectedId, host);
    });
  }
  addCommandAction(menu, "Draw Radiation", QString("ccx draw load radiation ") + QString::number(entityId));
}


void LoadsRadiationTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXLoadsRadiationModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<LoadsRadiationModifyPanel*>(widget))
    {
      panel->setRadiation(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Radiation modify panel.\n");
}

void LoadsRadiationTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXLoadsRadiationDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<LoadsRadiationDeletePanel*>(widget))
    {
      panel->setRadiation(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Radiation delete panel.\n");
}

void LoadsRadiationTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXLoadsRadiationCreate");
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
