#include "BCsTree.hpp"
#include "ModelTree.hpp"
#include <QMenu>
#include "CalculiXCoreInterface.hpp"

BCsTree::BCsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BCsTree::~BCsTree()
{}

void BCsTree::initialize()
{
  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"BCs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("BCsTree"));
  isInitialized = true;
}

void BCsTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    addCommandAction(menu, "Draw Bcs", QString("ccx draw bc_all"));
    return;
  }
}
