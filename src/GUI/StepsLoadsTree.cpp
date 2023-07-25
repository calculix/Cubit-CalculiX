#include "StepsLoadsTree.hpp"
#include "StepsLoadsForcesTree.hpp"
#include "StepsLoadsPressuresTree.hpp"
#include "StepsLoadsHeatfluxesTree.hpp"
#include "StepsLoadsGravityTree.hpp"
#include "StepsLoadsCentrifugalTree.hpp"
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
  if(isInitialized)
    return;

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
}

void StepsLoadsTree::update()
{
  myStepsLoadsForcesTree->update();
  myStepsLoadsPressuresTree->update();
  myStepsLoadsHeatfluxesTree->update();
  myStepsLoadsGravityTree->update();
  myStepsLoadsCentrifugalTree->update();
}

void StepsLoadsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}