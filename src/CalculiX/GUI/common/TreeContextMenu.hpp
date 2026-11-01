#ifndef TREECONTEXTMENU_HPP
#define TREECONTEXTMENU_HPP

#include <QString>

class QMenu;
class QTreeWidgetItem;
class ModelTree;

// Category items provide context menus for themselves and their entity rows.
class TreeContextMenu
{
public:
  virtual ~TreeContextMenu() = default;
  virtual void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) = 0;

protected:
  static bool readItemId(const QTreeWidgetItem *item, int &id);
  static void addPanelAction(
    QMenu &menu, const QString &label, ModelTree &host, const QString &marker);
  static void addCommandAction(
    QMenu &menu, const QString &label, const QString &command);
};

#endif // TREECONTEXTMENU_HPP
