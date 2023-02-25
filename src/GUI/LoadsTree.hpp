#ifndef LOADSTREE_HPP
#define LOADSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class LoadsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  LoadsTree(QTreeWidget *parent);
  ~LoadsTree();

  void initialize();

private:
  bool isInitialized;

  CalculiXCoreInterface *ccx_iface;
};

#endif // LOADSTREE_HPP
