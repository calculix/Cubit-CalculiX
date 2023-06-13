#include "LoadsTree.hpp"
#include "CalculiXCoreInterface.hpp"

LoadsTree::LoadsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

LoadsTree::~LoadsTree()
{
}

void LoadsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Loads");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("LoadsTree"));
  isInitialized = true;
}