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
  
  isInitialized = true;
}