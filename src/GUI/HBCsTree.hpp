#ifndef HBCSTREE_HPP
#define HBCSTREE_HPP

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class HBCsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  HBCsTree(QTreeWidget *parent);
  ~HBCsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  CalculiXCoreInterface *ccx_iface;
};

#endif // HBCSTREE_HPP