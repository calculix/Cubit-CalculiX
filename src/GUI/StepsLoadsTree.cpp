#include "StepsLoadsTree.hpp"
#include "StepsLoadsForcesTree.hpp"
#include "StepsLoadsPressuresTree.hpp"
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
}

void StepsLoadsTree::update()
{
  myStepsLoadsForcesTree->update();
  myStepsLoadsPressuresTree->update();
}

void StepsLoadsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}