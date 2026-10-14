#ifndef STEPSLOADSTREE_HPP
#define STEPSLOADSTREE_HPP

#include <QObject>
#include "TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;
class StepsLoadsForcesTree;
class StepsLoadsPressuresTree;
class StepsLoadsHeatfluxesTree;
class StepsLoadsGravityTree;
class StepsLoadsCentrifugalTree;
class StepsLoadsTrajectoryTree;
class StepsLoadsFilmTree;
class StepsLoadsRadiationTree;
class StepsLoadsSurfaceTractionTree;
class StepsLoadsBodyHeatfluxTree;

class StepsLoadsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  StepsLoadsTree(QTreeWidgetItem *parent);
  ~StepsLoadsTree();

  void initialize(int step_id_init);

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;
  int step_id = -1;

  CalculiXCoreInterface *ccx_iface;
  StepsLoadsForcesTree *myStepsLoadsForcesTree;
  StepsLoadsPressuresTree *myStepsLoadsPressuresTree;
  StepsLoadsHeatfluxesTree *myStepsLoadsHeatfluxesTree;
  StepsLoadsGravityTree *myStepsLoadsGravityTree;
  StepsLoadsCentrifugalTree *myStepsLoadsCentrifugalTree;
  StepsLoadsTrajectoryTree *myStepsLoadsTrajectoryTree;
  StepsLoadsFilmTree *myStepsLoadsFilmTree;
  StepsLoadsRadiationTree *myStepsLoadsRadiationTree;
  StepsLoadsSurfaceTractionTree *myStepsLoadsSurfaceTractionTree;
  StepsLoadsBodyHeatfluxTree *myStepsLoadsBodyHeatfluxTree;
};

#endif // STEPSLOADSTREE_HPP