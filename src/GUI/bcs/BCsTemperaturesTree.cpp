#include "BCsTemperaturesTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
BCsTemperaturesTree::BCsTemperaturesTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BCsTemperaturesTree::~BCsTemperaturesTree()
{
}

void BCsTemperaturesTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Temperatures");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("BCsTemperaturesTree"));
  isInitialized = true;
}

void BCsTemperaturesTree::update()
{
  CalculiXBCsInterface ccx_bcs_iface;

  std::vector<std::vector<std::string>> bcs_tree_data;
  bcs_tree_data = ccx_bcs_iface.get_bcstemperatures_tree_data();

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

void BCsTemperaturesTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void BCsTemperaturesTree::addBC(QString bc_id, QString bc_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *BCTreeChild = new QTreeWidgetItem();
  
  BCTreeChild->setText(0, bc_name);
    
  BCTreeChild->setText(1, bc_id);

  BCTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("BCsTemperaturesTree"));

  this->addChild(BCTreeChild);
}

void BCsTemperaturesTree::removeBC(QTreeWidgetItem *bc)
{
  this->removeChild(bc);
  delete bc;
}

int BCsTemperaturesTree::get_child_id(std::string bc_id)
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

void BCsTemperaturesTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    addPanelAction(menu, "Create Temperature", host, "FEATemperatureCreate");
    addCommandAction(menu, "Draw Temperatures", QString("ccx draw bc_temperature_all"));
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

  addPanelAction(menu, "Create Temperature", host, "FEATemperatureCreate");
  addPanelAction(menu, "Modify Temperature", host, "FEATemperatureModify");
  addPanelAction(menu, "CCX Modify Temperature", host, "CCXBCsTemperaturesModify");
  addPanelAction(menu, "Delete Temperature", host, "FEATemperatureDelete");
  addCommandAction(menu, "Draw Temperature", QString("ccx draw bc temperature ") + QString::number(entityId));
}
