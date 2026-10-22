#include "SidesetTree.hpp"

#include "GUI/application/ModelTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include <QMenu>
SidesetTree::SidesetTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

SidesetTree::~SidesetTree()
{
}

void SidesetTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Side Sets");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("SidesetTree"));
  isInitialized = true;
}

void SidesetTree::update()
{
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::vector<std::string>> sideset_tree_data;
  sideset_tree_data = ccx_mesh_iface.get_sideset_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < sideset_tree_data.size(); i++)
  {
    // check if sideset already exists as item
    
    ChildId = this->get_child_id(sideset_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addSideset(QString::fromStdString(sideset_tree_data[i][0]),
        QString::fromStdString(sideset_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = sideset_tree_data[i][1];    

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

    for (size_t ii = 0; ii < sideset_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==sideset_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeSideset(temp_child);
    }
  }
}

void SidesetTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void SidesetTree::addSideset(QString sideset_id, QString sideset_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *SidesetTreeChild = new QTreeWidgetItem();
  
  SidesetTreeChild->setText(0, sideset_name);
    
  SidesetTreeChild->setText(1, sideset_id);

  SidesetTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("SidesetTree"));

  this->addChild(SidesetTreeChild);
}

void SidesetTree::removeSideset(QTreeWidgetItem *sideset)
{
  this->removeChild(sideset);
  delete sideset;
}

int SidesetTree::get_child_id(std::string sideset_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==sideset_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void SidesetTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    addPanelAction(menu, "Create Sideset", host, "ExodusCreateSideset");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Sideset", host, "ExodusCreateSideset");
  addPanelAction(menu, "Modify Sideset", host, "ExodusRemoveContentsSideset");
  addPanelAction(menu, "Delete Sideset", host, "ExodusDeleteSideset");
}
