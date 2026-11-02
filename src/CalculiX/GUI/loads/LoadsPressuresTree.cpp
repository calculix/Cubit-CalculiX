#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "LoadsPressuresDeletePanel.hpp"
#include "LoadsPressuresModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "LoadsPressuresTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
LoadsPressuresTree::LoadsPressuresTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

LoadsPressuresTree::~LoadsPressuresTree()
{
}

void LoadsPressuresTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Pressures");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsPressuresTree"));
  isInitialized = true;
}

void LoadsPressuresTree::update()
{
  CalculiXLoadsInterface ccx_loads_iface;

  std::vector<std::vector<std::string>> loads_tree_data;
  loads_tree_data = ccx_loads_iface.get_loadspressures_tree_data();

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

void LoadsPressuresTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void LoadsPressuresTree::addLoad(QString load_id, QString load_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *LoadTreeChild = new QTreeWidgetItem();
  
  LoadTreeChild->setText(0, load_name);
    
  LoadTreeChild->setText(1, load_id);
  LoadTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Pressure));
  LoadTreeChild->setData(0, PropertyIdRole, load_id.toInt());

  LoadTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsPressuresTree"));

  this->addChild(LoadTreeChild);
}

void LoadsPressuresTree::removeLoad(QTreeWidgetItem *load)
{
  this->removeChild(load);
  delete load;
}

int LoadsPressuresTree::get_child_id(std::string load_id)
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

void LoadsPressuresTree::populateContextMenu(
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
    addPanelAction(menu, "Create Pressure", host, "CCXLoadsPressuresCreate");
    addCommandAction(menu, "Draw Pressures", QString("ccx draw load_pressure_all"));
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

  addPanelAction(menu, "Create Pressure", host, "CCXLoadsPressuresCreate");
  {
    auto* action = menu.addAction("Modify Pressure");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      LoadsPressuresTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Pressure");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      LoadsPressuresTree::open_delete(selectedId, host);
    });
  }
  addCommandAction(menu, "Draw Pressure", QString("ccx draw load pressure ") + QString::number(entityId));
}


void LoadsPressuresTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXLoadsPressuresModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<LoadsPressuresModifyPanel*>(widget))
    {
      panel->setPressure(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Pressure modify panel.\n");
}

void LoadsPressuresTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXLoadsPressuresDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<LoadsPressuresDeletePanel*>(widget))
    {
      panel->setPressure(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Pressure delete panel.\n");
}

void LoadsPressuresTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXLoadsPressuresCreate");
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
