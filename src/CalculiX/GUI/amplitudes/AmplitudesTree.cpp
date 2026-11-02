#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "AmplitudesDeletePanel.hpp"
#include "AmplitudesModifyPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "AmplitudesTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
AmplitudesTree::AmplitudesTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

AmplitudesTree::~AmplitudesTree()
{
}

void AmplitudesTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Amplitudes");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("AmplitudesTree"));
  isInitialized = true;
}

void AmplitudesTree::update()
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;

  std::vector<std::vector<std::string>> amplitudes_tree_data;
  amplitudes_tree_data = ccx_amplitudes_iface.get_amplitudes_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < amplitudes_tree_data.size(); i++)
  {
    // check if amplitude already exists as item
    
    ChildId = this->get_child_id(amplitudes_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addAmplitude(QString::fromStdString(amplitudes_tree_data[i][0]),
        QString::fromStdString(amplitudes_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = amplitudes_tree_data[i][1];    

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

    for (size_t ii = 0; ii < amplitudes_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==amplitudes_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeAmplitude(temp_child);
    }
  }
}

void AmplitudesTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void AmplitudesTree::addAmplitude(QString amplitude_id, QString amplitude_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *AmplitudeTreeChild = new QTreeWidgetItem();
  
  AmplitudeTreeChild->setText(0, amplitude_name);
    
  AmplitudeTreeChild->setText(1, amplitude_id);
  AmplitudeTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Amplitude));
  AmplitudeTreeChild->setData(0, PropertyIdRole, amplitude_id.toInt());

  AmplitudeTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("AmplitudesTree"));

  this->addChild(AmplitudeTreeChild);
}

void AmplitudesTree::removeAmplitude(QTreeWidgetItem *amplitude)
{
  this->removeChild(amplitude);
  delete amplitude;
}

int AmplitudesTree::get_child_id(std::string amplitude_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==amplitude_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void AmplitudesTree::populateContextMenu(
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
    addPanelAction(menu, "Create Amplitude", host, "CCXAmplitudesCreate");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Amplitude", host, "CCXAmplitudesCreate");
  {
    auto* action = menu.addAction("Modify Amplitude");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      AmplitudesTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Amplitude");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      AmplitudesTree::open_delete(selectedId, host);
    });
  }
}


void AmplitudesTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXAmplitudesModify");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<AmplitudesModifyPanel*>(widget))
    {
      panel->setAmplitude(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Amplitude modify panel.\n");
}

void AmplitudesTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXAmplitudesDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<AmplitudesDeletePanel*>(widget))
    {
      panel->setAmplitude(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Amplitude delete panel.\n");
}

void AmplitudesTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXAmplitudesCreate");
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
