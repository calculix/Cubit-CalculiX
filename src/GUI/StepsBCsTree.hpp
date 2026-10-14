#ifndef STEPSBCSTREE_HPP
#define STEPSBCSTREE_HPP

#include <QObject>
#include "TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;
class StepsBCsDisplacementsTree;
class StepsBCsTemperaturesTree;

class StepsBCsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  StepsBCsTree(QTreeWidgetItem *parent);
  ~StepsBCsTree();

  void initialize(int step_id_init);

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;
  int step_id = -1;

  CalculiXCoreInterface *ccx_iface;
  StepsBCsDisplacementsTree *myStepsBCsDisplacementsTree;
  StepsBCsTemperaturesTree *myStepsBCsTemperaturesTree;
};

#endif // STEPSBCSTREE_HPP