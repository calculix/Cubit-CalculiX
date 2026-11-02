#include "GUI/properties/PropertySelection.hpp"
#include "StepsDeletePanel.hpp"
#include "StepsModifyComplexFrequencyPanel.hpp"
#include "StepsModifySteadyStateDynamicsPanel.hpp"
#include "StepsModifyModalDynamicPanel.hpp"
#include "StepsModifyDynamicPanel.hpp"
#include "StepsModifyUncoupledTemperatureDisplacementPanel.hpp"
#include "StepsModifyCoupledTemperatureDisplacementPanel.hpp"
#include "StepsModifyHeatTransferPanel.hpp"
#include "StepsModifyBucklePanel.hpp"
#include "StepsModifyFrequencyPanel.hpp"
#include "StepsModifyStaticPanel.hpp"
#include "StepsModifyParameterPanel.hpp"
#include "TaskPanel.hpp"
#include "StepsTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include "CubitMessage.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include <QMenu>
#include <QAction>
#include "StepsLoadsTree.hpp"
#include "StepsBCsTree.hpp"
#include "StepsHistoryOutputsTree.hpp"
#include "StepsFieldOutputsTree.hpp"
StepsTree::StepsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsTree::~StepsTree()
{}

void StepsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Steps");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("StepsTree"));
  isInitialized = true;
}

void StepsTree::update()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::vector<std::string>> steps_tree_data;
  steps_tree_data = ccx_steps_iface.get_steps_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < steps_tree_data.size(); i++)
  {
    // check if step already exists as item
    
    ChildId = this->get_child_id(steps_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addStep(QString::fromStdString(steps_tree_data[i][0]),
        QString::fromStdString(steps_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      // update childs
      StepsLoadsTree *StepsLoadsChild;
      StepsLoadsChild = dynamic_cast<StepsLoadsTree*>(temp_child->child(0));
      StepsLoadsChild->update();
      StepsBCsTree *StepsBCsChild;
      StepsBCsChild = dynamic_cast<StepsBCsTree*>(temp_child->child(1));
      StepsBCsChild->update();
      StepsHistoryOutputsTree *StepsHistoryOutputsChild;
      StepsHistoryOutputsChild = dynamic_cast<StepsHistoryOutputsTree*>(temp_child->child(2));
      StepsHistoryOutputsChild->update();
      StepsFieldOutputsTree *StepsFieldOutputsChild;
      StepsFieldOutputsChild = dynamic_cast<StepsFieldOutputsTree*>(temp_child->child(3));
      StepsFieldOutputsChild->update();

      str_check = steps_tree_data[i][1];    

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

    for (size_t ii = 0; ii < steps_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==steps_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeStep(temp_child);
    }
  }
}

void StepsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void StepsTree::addStep(QString step_id, QString step_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *StepTreeChild = new QTreeWidgetItem();
  
  StepTreeChild->setText(0, step_name);
    
  StepTreeChild->setText(1, step_id);
  StepTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Step));
  StepTreeChild->setData(0, PropertyIdRole, step_id.toInt());
  StepTreeChild->setData(0, PropertyStepIdRole, step_id.toInt());

  StepTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("StepsTree"));

  StepsLoadsTree *StepsLoadsChild = new StepsLoadsTree(StepTreeChild);
  StepsLoadsChild->initialize(step_id.toInt());

  StepsBCsTree *StepsBCsChild = new StepsBCsTree(StepTreeChild);
  StepsBCsChild->initialize(step_id.toInt());

  StepsHistoryOutputsTree *StepsHistoryOutputsChild = new StepsHistoryOutputsTree(StepTreeChild);
  StepsHistoryOutputsChild->initialize(step_id.toInt());

  StepsFieldOutputsTree *StepsFieldOutputsChild = new StepsFieldOutputsTree(StepTreeChild);
  StepsFieldOutputsChild->initialize(step_id.toInt());

  this->addChild(StepTreeChild);
}

void StepsTree::removeStep(QTreeWidgetItem *step)
{
  this->removeChild(step);
  delete step;
}

int StepsTree::get_child_id(std::string step_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==step_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void StepsTree::populateContextMenu(
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
    addPanelAction(menu, "Create Step", host, "CCXStepsCreate");
    {
      QAction *action = menu.addAction("Draw Steps");
      ModelTree *hostPtr = &host;
      QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
      {
        CalculiXStepsInterface ccx_steps_iface;


        std::vector<int> step_ids;
        step_ids = ccx_steps_iface.get_steps_ids();
        for (size_t i=0; i < step_ids.size(); i++)
        {
          std::string command = "ccx draw step " + std::to_string(step_ids[i]);

          GuiSendCmd::gui_cmd(QString::fromStdString(command));
        }
      });
    }
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

  addPanelAction(menu, "Create Step", host, "CCXStepsCreate");
  {
    auto* action = menu.addAction("Modify Step");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      StepsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Step");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      StepsTree::open_delete(selectedId, host);
    });
  }
  addCommandAction(menu, "Draw Step", QString("ccx draw step ") + QString::number(entityId));
}


void StepsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  CalculiXStepsInterface core;
  StepsSettings settings;
  std::string error;
  if (!core.get_step_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot open Step %d: %s\n", id, error.c_str());
    return;
  }

  QString marker;
  switch (settings.type)
  {
    case 1: marker = "CCXStepsModifyParameter"; break;
    case 2: marker = "CCXStepsModifyStatic"; break;
    case 3: marker = "CCXStepsModifyFrequency"; break;
    case 4: marker = "CCXStepsModifyBuckle"; break;
    case 5: marker = "CCXStepsModifyHeatTransfer"; break;
    case 6: marker = "CCXStepsModifyCoupledTemperatureDisplacement"; break;
    case 7: marker = "CCXStepsModifyUncoupledTemperatureDisplacement"; break;
    case 8: marker = "CCXStepsModifyDynamic"; break;
    case 9: marker = "CCXStepsModifyModalDynamic"; break;
    case 10: marker = "CCXStepsModifySteadyStateDynamics"; break;
    case 11: marker = "CCXStepsModifyComplexFrequency"; break;
    default:
      PRINT_ERROR("This stored type has no modify panel.\n");
      return;
  }
  host.setWidgetInCmdPanelMarker(marker);

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<StepsModifyParameterPanel*>(widget))
    {
      if (settings.type == 1)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyStaticPanel*>(widget))
    {
      if (settings.type == 2)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyFrequencyPanel*>(widget))
    {
      if (settings.type == 3)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyBucklePanel*>(widget))
    {
      if (settings.type == 4)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyHeatTransferPanel*>(widget))
    {
      if (settings.type == 5)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyCoupledTemperatureDisplacementPanel*>(widget))
    {
      if (settings.type == 6)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyUncoupledTemperatureDisplacementPanel*>(widget))
    {
      if (settings.type == 7)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyDynamicPanel*>(widget))
    {
      if (settings.type == 8)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyModalDynamicPanel*>(widget))
    {
      if (settings.type == 9)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifySteadyStateDynamicsPanel*>(widget))
    {
      if (settings.type == 10)
      {
        panel->setStep(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<StepsModifyComplexFrequencyPanel*>(widget))
    {
      if (settings.type == 11)
      {
        panel->setStep(id);
        return;
      }
    }
  }
  PRINT_ERROR("Unable to open the Step modify panel.\n");
}

void StepsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXStepsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<StepsDeletePanel*>(widget))
    {
      panel->setStep(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Step delete panel.\n");
}

void StepsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXStepsCreate");
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
