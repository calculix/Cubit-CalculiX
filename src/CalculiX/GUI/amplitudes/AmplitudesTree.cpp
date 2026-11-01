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
  addPanelAction(menu, "Modify Amplitude", host, "CCXAmplitudesModify");
  addPanelAction(menu, "Delete Amplitude", host, "CCXAmplitudesDelete");
}
