#include "StepsLoadsTree.hpp"
#include "ModelTree.hpp"
#include "CubitMessage.hpp"
#include "StepsManagement.hpp"
#include <QMenu>
#include <QAction>
#include "StepsLoadsForcesTree.hpp"
#include "StepsLoadsPressuresTree.hpp"
#include "StepsLoadsHeatfluxesTree.hpp"
#include "StepsLoadsGravityTree.hpp"
#include "StepsLoadsCentrifugalTree.hpp"
#include "StepsLoadsTrajectoryTree.hpp"
#include "StepsLoadsFilmTree.hpp"
#include "StepsLoadsRadiationTree.hpp"
#include "StepsLoadsSurfaceTractionTree.hpp"
#include "StepsLoadsBodyHeatfluxTree.hpp"
#include "CalculiXCoreInterface.hpp"

StepsLoadsTree::StepsLoadsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsLoadsTree::~StepsLoadsTree()
{
}

void StepsLoadsTree::initialize(int step_id_init)
{
  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Loads");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("StepsLoadsTree"));  
  
  isInitialized = true;
  step_id = step_id_init;

  myStepsLoadsForcesTree = new StepsLoadsForcesTree(this);
  myStepsLoadsForcesTree->initialize(step_id);
  myStepsLoadsPressuresTree = new StepsLoadsPressuresTree(this);
  myStepsLoadsPressuresTree->initialize(step_id);
  myStepsLoadsHeatfluxesTree = new StepsLoadsHeatfluxesTree(this);
  myStepsLoadsHeatfluxesTree->initialize(step_id);
  myStepsLoadsGravityTree = new StepsLoadsGravityTree(this);
  myStepsLoadsGravityTree->initialize(step_id);
  myStepsLoadsCentrifugalTree = new StepsLoadsCentrifugalTree(this);
  myStepsLoadsCentrifugalTree->initialize(step_id);
  myStepsLoadsTrajectoryTree = new StepsLoadsTrajectoryTree(this);
  myStepsLoadsTrajectoryTree->initialize(step_id);
  myStepsLoadsFilmTree = new StepsLoadsFilmTree(this);
  myStepsLoadsFilmTree->initialize(step_id);
  myStepsLoadsRadiationTree = new StepsLoadsRadiationTree(this);
  myStepsLoadsRadiationTree->initialize(step_id);
  myStepsLoadsSurfaceTractionTree = new StepsLoadsSurfaceTractionTree(this);
  myStepsLoadsSurfaceTractionTree->initialize(step_id);
  myStepsLoadsBodyHeatfluxTree = new StepsLoadsBodyHeatfluxTree(this);
  myStepsLoadsBodyHeatfluxTree->initialize(step_id);
}

void StepsLoadsTree::update()
{
  myStepsLoadsForcesTree->update();
  myStepsLoadsPressuresTree->update();
  myStepsLoadsHeatfluxesTree->update();
  myStepsLoadsGravityTree->update();
  myStepsLoadsCentrifugalTree->update();
  myStepsLoadsTrajectoryTree->update();
  myStepsLoadsFilmTree->update();
  myStepsLoadsRadiationTree->update();
  myStepsLoadsSurfaceTractionTree->update();
  myStepsLoadsBodyHeatfluxTree->update();
}

void StepsLoadsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void StepsLoadsTree::populateContextMenu(
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
    addCommandAction(menu, "Draw Loads", QString("ccx draw step ") + QString::number(step_id) + " load_all");
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
