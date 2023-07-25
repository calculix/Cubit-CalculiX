#include "StepsBCsTree.hpp"
#include "StepsBCsDisplacementsTree.hpp"
#include "StepsBCsTemperaturesTree.hpp"
#include "CalculiXCoreInterface.hpp"

StepsBCsTree::StepsBCsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsBCsTree::~StepsBCsTree()
{
}

void StepsBCsTree::initialize(int step_id_init)
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"BCs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("StepsBCsTree"));
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