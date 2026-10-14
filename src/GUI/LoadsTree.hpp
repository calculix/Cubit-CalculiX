#ifndef LOADSTREE_HPP
#define LOADSTREE_HPP

#include <QObject>
#include "TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

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

  CalculiXCoreInterface *ccx_iface;
};

#endif // LOADSTREE_HPP
