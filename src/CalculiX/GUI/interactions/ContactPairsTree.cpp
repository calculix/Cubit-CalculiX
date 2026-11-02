#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "ContactPairsDeletePanel.hpp"
#include "ContactPairsModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "ContactPairsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
ContactPairsTree::ContactPairsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

ContactPairsTree::~ContactPairsTree()
{
}

void ContactPairsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Contact Pairs");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("ContactPairsTree"));
  isInitialized = true;
}

void ContactPairsTree::update()
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  std::vector<std::vector<std::string>> contactpairs_tree_data;
  contactpairs_tree_data = ccx_interactions_iface.get_contactpairs_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < contactpairs_tree_data.size(); i++)
  {
    // check if contactpair already exists as item
    
    ChildId = this->get_child_id(contactpairs_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addContactPair(QString::fromStdString(contactpairs_tree_data[i][0]),
        QString::fromStdString(contactpairs_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = contactpairs_tree_data[i][1];    

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

    for (size_t ii = 0; ii < contactpairs_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==contactpairs_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeContactPair(temp_child);
    }
  }
}

void ContactPairsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void ContactPairsTree::addContactPair(QString contactpair_id, QString contactpair_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *ContactPairTreeChild = new QTreeWidgetItem();
  
  ContactPairTreeChild->setText(0, contactpair_name);
    
  ContactPairTreeChild->setText(1, contactpair_id);
  ContactPairTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::ContactPair));
  ContactPairTreeChild->setData(0, PropertyIdRole, contactpair_id.toInt());

  ContactPairTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("ContactPairsTree"));

  this->addChild(ContactPairTreeChild);
}

void ContactPairsTree::removeContactPair(QTreeWidgetItem *contactpair)
{
  this->removeChild(contactpair);
  delete contactpair;
}

int ContactPairsTree::get_child_id(std::string contactpair_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==contactpair_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void ContactPairsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Contact Pair", host, "CCXContactPairsCreate");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Contact Pair", host, "CCXContactPairsCreate");
  {
    auto* action = menu.addAction("Modify Contact Pair");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      ContactPairsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Contact Pair");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      ContactPairsTree::open_delete(selectedId, host);
    });
  }
}


void ContactPairsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXContactPairsModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<ContactPairsModifyPanel*>(widget))
    {
      panel->setContactPair(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the ContactPair modify panel.\n");
}

void ContactPairsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXContactPairsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<ContactPairsDeletePanel*>(widget))
    {
      panel->setContactPair(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the ContactPair delete panel.\n");
}

void ContactPairsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXContactPairsCreate");
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
