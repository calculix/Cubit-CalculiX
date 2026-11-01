#include "LoadsTree.hpp"
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include <QMenu>
LoadsTree::LoadsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

LoadsTree::~LoadsTree()
{
}

void LoadsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Loads");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("LoadsTree"));
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
