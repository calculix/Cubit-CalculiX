#include "BCsTree.hpp"
#include "CalculiXCoreInterface.hpp"

BCsTree::BCsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BCsTree::~BCsTree()
{}

void BCsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"BCs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}