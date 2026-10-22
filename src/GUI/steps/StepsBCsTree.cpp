#include "StepsBCsTree.hpp"
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include "CubitMessage.hpp"
#include "StepsManagement.hpp"
#include <QMenu>
#include <QAction>
#include "StepsBCsDisplacementsTree.hpp"
#include "StepsBCsTemperaturesTree.hpp"
StepsBCsTree::StepsBCsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsBCsTree::~StepsBCsTree()
{
}

void StepsBCsTree::initialize(int step_id_init)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"BCs");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("StepsBCsTree"));
  isInitialized = true;
  step_id = step_id_init;

  myStepsBCsDisplacementsTree = new StepsBCsDisplacementsTree(this);
  myStepsBCsDisplacementsTree->initialize(step_id);
  myStepsBCsTemperaturesTree = new StepsBCsTemperaturesTree(this);
  myStepsBCsTemperaturesTree->initialize(step_id);
}

void StepsBCsTree::update()
{
  myStepsBCsDisplacementsTree->update();
  myStepsBCsTemperaturesTree->update();
}

void StepsBCsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void StepsBCsTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    if (step_id < 0)
    {
      PRINT_WARNING("The model tree category has no valid step ID.\n");
      return;
    }

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
    addCommandAction(menu, "Draw BCs", QString("ccx draw step ") + QString::number(step_id) + " bc_all");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

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
