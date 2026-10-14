#ifndef BCSTREE_HPP
#define BCSTREE_HPP

#include <QObject>
#include "TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class BCsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  BCsTree(QTreeWidget *parent);
  ~BCsTree();

  void initialize();

private:
  bool isInitialized;

  CalculiXCoreInterface *ccx_iface;
};

#endif // BCSTREE_HPP