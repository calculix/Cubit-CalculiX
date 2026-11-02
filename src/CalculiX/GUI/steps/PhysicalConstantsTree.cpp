#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "PhysicalConstantsDeletePanel.hpp"
#include "PhysicalConstantsModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "PhysicalConstantsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
PhysicalConstantsTree::PhysicalConstantsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

PhysicalConstantsTree::~PhysicalConstantsTree()
{
}

void PhysicalConstantsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Physical Constants");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("PhysicalConstantsTree"));
  setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::PhysicalConstants));
  setData(0, PropertyIdRole, 0);
  isInitialized = true;
}

void PhysicalConstantsTree::update()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::vector<std::string>> physicalconstants_tree_data;
  physicalconstants_tree_data = ccx_steps_iface.get_physicalconstants_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < physicalconstants_tree_data.size(); i++)
  {
    // check if PhysicalConstants already exists as item
    
    ChildId = this->get_child_id(physicalconstants_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addPhysicalConstants(QString::fromStdString(physicalconstants_tree_data[i][0]),
        QString::fromStdString(physicalconstants_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = physicalconstants_tree_data[i][1];    

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

    for (size_t ii = 0; ii < physicalconstants_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==physicalconstants_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removePhysicalConstants(temp_child);
    }
  }
}

void PhysicalConstantsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void PhysicalConstantsTree::addPhysicalConstants(QString PhysicalConstants_id, QString name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *PhysicalConstantsTreeChild = new QTreeWidgetItem();
  
  PhysicalConstantsTreeChild->setText(0, name);
    
  PhysicalConstantsTreeChild->setText(1, PhysicalConstants_id);
  PhysicalConstantsTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::PhysicalConstants));
  PhysicalConstantsTreeChild->setData(0, PropertyIdRole, PhysicalConstants_id.toInt());

  PhysicalConstantsTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("PhysicalConstantsTree"));

  this->addChild(PhysicalConstantsTreeChild);
}

void PhysicalConstantsTree::removePhysicalConstants(QTreeWidgetItem *PhysicalConstants)
{
  this->removeChild(PhysicalConstants);
  delete PhysicalConstants;
}

int PhysicalConstantsTree::get_child_id(std::string PhysicalConstants_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==PhysicalConstants_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void PhysicalConstantsTree::populateContextMenu(
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
    {
      auto* action = menu.addAction("Modify PhysicalConstants");
      QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
      {
        PhysicalConstantsTree::open_modify(0, host);
      });
    }
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  {
    auto* action = menu.addAction("Modify PhysicalConstants");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      PhysicalConstantsTree::open_modify(0, host);
    });
  }
  {
    auto* action = menu.addAction("Delete PhysicalConstants");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      PhysicalConstantsTree::open_delete(selectedId, host);
    });
  }
}


void PhysicalConstantsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXPhysicalConstantsModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<PhysicalConstantsModifyPanel*>(widget))
    {
      panel->setPhysicalConstants(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the PhysicalConstants modify panel.\n");
}

void PhysicalConstantsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXPhysicalConstantsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<PhysicalConstantsDeletePanel*>(widget))
    {
      panel->selectConstant(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the PhysicalConstants delete panel.\n");
}

void PhysicalConstantsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  open_modify(0, host);
}
