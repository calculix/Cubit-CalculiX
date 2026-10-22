#ifndef HBCSTREE_HPP
#define HBCSTREE_HPP

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QIcon>
class HBCsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  HBCsTree(QTreeWidget *parent);
  ~HBCsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

};

#endif // HBCSTREE_HPP