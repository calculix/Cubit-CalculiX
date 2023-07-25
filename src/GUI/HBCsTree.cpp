#include "HBCsTree.hpp"
#include "CalculiXCoreInterface.hpp"

HBCsTree::HBCsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

HBCsTree::~HBCsTree()
{
}

void HBCsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"homogeneous BCs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("HBCsTree"));
  isInitialized = true;
}

void HBCsTree::update()
{}

void HBCsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}