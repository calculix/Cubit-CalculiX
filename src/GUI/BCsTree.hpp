#ifndef BCSTREE_HPP
#define BCSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class BCsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  BCsTree(QTreeWidget *parent);
  ~BCsTree();

  void initialize();

private:
  bool isInitialized;

  CalculiXCoreInterface *ccx_iface;
};

#endif // BCSTREE_HPP