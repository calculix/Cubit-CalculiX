#include "TreeContextMenu.hpp"
#include "GUI/application/ModelTree.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "CubitMessage.hpp"

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QTreeWidgetItem>

bool TreeContextMenu::readItemId(const QTreeWidgetItem *item, int &id)
{
  id = -1;
  if (item == nullptr)
  {
    PRINT_WARNING("Cannot read the model tree item ID.\n");
    return false;
  }

  bool valid = false;
  const int parsedId = item->text(1).toInt(&valid);
  if (!valid || parsedId < 0)
  {
    PRINT_WARNING("The model tree item has an invalid ID.\n");
    return false;
  }

  id = parsedId;
  return true;
}

void TreeContextMenu::addPanelAction(
  QMenu &menu, const QString &label, ModelTree &host, const QString &marker)
{
  QAction *action = menu.addAction(label);
  ModelTree *hostPtr = &host;
  QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr, marker]()
  {
    hostPtr->setWidgetInCmdPanelMarker(marker);
  });
}

void TreeContextMenu::addCommandAction(
  QMenu &menu, const QString &label, const QString &command)
{
  QAction *action = menu.addAction(label);
  QObject::connect(action, &QAction::triggered, &menu, [command]()
  {
    GuiSendCmd::gui_cmd(command);
  });
}
