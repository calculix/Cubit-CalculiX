#include "BCsTree.hpp"
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
BCsTree::BCsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

BCsTree::~BCsTree()
{}

void BCsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"BCs");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("BCsTree"));
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
