#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "FieldOutputsDeletePanel.hpp"
#include "FieldOutputsModifyContactPanel.hpp"
#include "FieldOutputsModifyElementPanel.hpp"
#include "FieldOutputsModifyNodePanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "FieldOutputsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
FieldOutputsTree::FieldOutputsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

FieldOutputsTree::~FieldOutputsTree()
{
}

void FieldOutputsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Field Outputs");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("FieldOutputsTree"));
  isInitialized = true;
}

void FieldOutputsTree::update()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::vector<std::string>> outputs_tree_data;
  outputs_tree_data = ccx_steps_iface.get_fieldoutputs_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < outputs_tree_data.size(); i++)
  {
    // check if output already exists as item
    
    ChildId = this->get_child_id(outputs_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addOutput(QString::fromStdString(outputs_tree_data[i][0]),
        QString::fromStdString(outputs_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = outputs_tree_data[i][1];    

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

    for (size_t ii = 0; ii < outputs_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==outputs_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeOutput(temp_child);
    }
  }
}

void FieldOutputsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void FieldOutputsTree::addOutput(QString output_id, QString output_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *OutputTreeChild = new QTreeWidgetItem();
  
  OutputTreeChild->setText(0, output_name);
    
  OutputTreeChild->setText(1, output_id);
  OutputTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::FieldOutput));
  OutputTreeChild->setData(0, PropertyIdRole, output_id.toInt());

  OutputTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("FieldOutputsTree"));

  this->addChild(OutputTreeChild);
}

void FieldOutputsTree::removeOutput(QTreeWidgetItem *output)
{
  this->removeChild(output);
  delete output;
}

int FieldOutputsTree::get_child_id(std::string output_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==output_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void FieldOutputsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Field Output", host, "CCXFieldOutputsCreate");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Field Output", host, "CCXFieldOutputsCreate");
  {
    auto* action = menu.addAction("Modify Field Output");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      FieldOutputsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Field Output");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      FieldOutputsTree::open_delete(selectedId, host);
    });
  }
}


void FieldOutputsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  CalculiXStepsInterface core;
  FieldOutputsSettings settings;
  std::string error;
  if (!core.get_fieldoutput_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot open FieldOutput %d: %s\n", id, error.c_str());
    return;
  }

  QString marker;
  switch (settings.type)
  {
    case 1: marker = "CCXFieldOutputsModifyNode"; break;
    case 2: marker = "CCXFieldOutputsModifyElement"; break;
    case 3: marker = "CCXFieldOutputsModifyContact"; break;
    default:
      PRINT_ERROR("This stored type has no modify panel.\n");
      return;
  }
  host.setWidgetInCmdPanelMarker(marker);

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<FieldOutputsModifyNodePanel*>(widget))
    {
      if (settings.type == 1)
      {
        panel->setFieldOutput(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<FieldOutputsModifyElementPanel*>(widget))
    {
      if (settings.type == 2)
      {
        panel->setFieldOutput(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<FieldOutputsModifyContactPanel*>(widget))
    {
      if (settings.type == 3)
      {
        panel->setFieldOutput(id);
        return;
      }
    }
  }
  PRINT_ERROR("Unable to open the FieldOutput modify panel.\n");
}

void FieldOutputsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXFieldOutputsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<FieldOutputsDeletePanel*>(widget))
    {
      panel->setFieldOutput(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the FieldOutput delete panel.\n");
}

void FieldOutputsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXFieldOutputsCreate");
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
