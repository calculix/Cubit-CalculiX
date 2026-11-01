#ifndef LOADSTREE_HPP
#define LOADSTREE_HPP

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class LoadsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  LoadsTree(QTreeWidget *parent);
  ~LoadsTree();

  void initialize();

private:
  bool isInitialized;

};

#endif // LOADSTREE_HPP
