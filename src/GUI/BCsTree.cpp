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
  this->setIcon(0,ccx_iface->getIcon2("BCsTree"));
  isInitialized = true;
}