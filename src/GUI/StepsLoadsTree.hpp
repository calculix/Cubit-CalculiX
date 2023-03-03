#ifndef STEPSLOADSTREE_HPP
#define STEPSLOADSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class StepsLoadsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  StepsLoadsTree(QTreeWidgetItem *parent);
  ~StepsLoadsTree();

  void initialize();

private:
  bool isInitialized;

  CalculiXCoreInterface *ccx_iface;
};

#endif // STEPSLOADSTREE_HPP