#include "LoadsTree.hpp"
#include "ModelTree.hpp"
#include <QMenu>
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
  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Loads");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("LoadsTree"));
  isInitialized = true;
}

void LoadsTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    addCommandAction(menu, "Draw Loads", QString("ccx draw load_all"));
    return;
  }
}
