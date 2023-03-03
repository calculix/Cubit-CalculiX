#include "StepsLoadsTree.hpp"
#include "CalculiXCoreInterface.hpp"

StepsLoadsTree::StepsLoadsTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsLoadsTree::~StepsLoadsTree()
{
}

void StepsLoadsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Loads");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}