#include "StepsHistoryOutputsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include "CubitMessage.hpp"
#include "StepsManagement.hpp"
#include <QMenu>
#include <QAction>
StepsHistoryOutputsTree::StepsHistoryOutputsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsHistoryOutputsTree::~StepsHistoryOutputsTree()
{
}

void StepsHistoryOutputsTree::initialize(int step_id_init)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"History Outputs");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("StepsHistoryOutputsTree"));
  isInitialized = true;
  step_id = step_id_init;
}

void StepsHistoryOutputsTree::update()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::vector<std::string>> outputs_tree_data;
  outputs_tree_data = ccx_steps_iface.get_steps_historyoutputs_tree_data(step_id);

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

void StepsHistoryOutputsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void StepsHistoryOutputsTree::addOutput(QString output_id, QString output_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *OutputTreeChild = new QTreeWidgetItem();
  
  OutputTreeChild->setText(0, output_name);
    
  OutputTreeChild->setText(1, output_id);

  OutputTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("StepsHistoryOutputsTree"));

  this->addChild(OutputTreeChild);
}

void StepsHistoryOutputsTree::removeOutput(QTreeWidgetItem *output)
{
  this->removeChild(output);
  delete output;
}

int StepsHistoryOutputsTree::get_child_id(std::string output_id)
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

void StepsHistoryOutputsTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    {
      QAction *action = menu.addAction("Steps Management");
      ModelTree *hostPtr = &host;
      QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
      {
        if (hostPtr->myStepsManagement == nullptr)
        {
          PRINT_WARNING("The requested model tree service is unavailable.\n");
          return;
        }

        hostPtr->myStepsManagement->show();
      });
    }
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create History Output", host, "CCXHistoryOutputsCreate");
  addPanelAction(menu, "Modify History Output", host, "CCXHistoryOutputsModify");
  addPanelAction(menu, "Delete History Output", host, "CCXHistoryOutputsDelete");
  {
    QAction *action = menu.addAction("Steps Management");
    ModelTree *hostPtr = &host;
    QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
    {
      if (hostPtr->myStepsManagement == nullptr)
      {
        PRINT_WARNING("The requested model tree service is unavailable.\n");
        return;
      }

      hostPtr->myStepsManagement->show();
    });
  }
}
