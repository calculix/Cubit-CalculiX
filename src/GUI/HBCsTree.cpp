#include "HBCsTree.hpp"
#include "ModelTree.hpp"
#include "CubitMessage.hpp"
#include "HBCsManagement.hpp"
#include <QMenu>
#include <QAction>
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
  if (isInitialized)
  {
    return;
  }

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
