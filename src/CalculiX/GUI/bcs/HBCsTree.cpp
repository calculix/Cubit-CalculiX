#include "GUI/properties/PropertySelection.hpp"
#include "HBCsTree.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include "CubitMessage.hpp"
#include "HBCsManagement.hpp"
#include <QMenu>
#include <QAction>
HBCsTree::HBCsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

HBCsTree::~HBCsTree()
{
}

void HBCsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"homogeneous BCs");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("HBCsTree"));
  setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::HBCs));
  setData(0, PropertyIdRole, 0);
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

void HBCsTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    {
      QAction *action = menu.addAction("homogeneous BCs Management");
      ModelTree *hostPtr = &host;
      QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
      {
        if (hostPtr->myHBCsManagement == nullptr)
        {
          PRINT_WARNING("The requested model tree service is unavailable.\n");
          return;
        }

        hostPtr->myHBCsManagement->show();
      });
    }
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  {
    QAction *action = menu.addAction("homogeneous BCs Management");
    ModelTree *hostPtr = &host;
    QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
    {
      if (hostPtr->myHBCsManagement == nullptr)
      {
        PRINT_WARNING("The requested model tree service is unavailable.\n");
        return;
      }

      hostPtr->myHBCsManagement->show();
    });
  }
}


void HBCsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (host.myHBCsManagement == nullptr)
  {
    return;
  }
  host.myHBCsManagement->update();
  host.myHBCsManagement->show();
  host.myHBCsManagement->raise();
}
