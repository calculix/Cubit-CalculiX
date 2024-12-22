#ifndef MODELTREE_HPP
#define MODELTREE_HPP

#include <QWidget>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QModelIndex>

class BlocksTree;
class NodesetTree;
class SidesetTree;
class MaterialTree;
class MaterialManagement;
class SectionsTree;
class ConstraintsTree;
class SurfaceInteractionsTree;
class ContactPairsTree;
class AmplitudesTree;
class OrientationsTree;
class DampingTree;
class PhysicalConstantsTree;
class LoadsTree;
class LoadsForcesTree;
class LoadsPressuresTree;
class LoadsHeatfluxesTree;
class LoadsGravityTree;
class LoadsCentrifugalTree;
class LoadsTrajectoryTree;
class LoadsFilmTree;
class LoadsRadiationTree;
class BCsTree;
class BCsDisplacementsTree;
class BCsTemperaturesTree;
class HistoryOutputsTree;
class FieldOutputsTree;
class InitialConditionsTree;
class HBCsTree;
class HBCsDisplacementsTree;
class HBCsTemperaturesTree;
class HBCsManagement;
class StepsTree;
class StepsLoadsTree;
class StepsLoadsForcesTree;
class StepsLoadsPressuresTree;
class StepsLoadsHeatfluxesTree;
class StepsLoadsGravityTree;
class StepsLoadsCentrifugalTree;
class StepsLoadsTrajectoryTree;
class StepsLoadsFilmTree;
class StepsLoadsRadiationTree;
class StepsBCsTree;
class StepsBCsDisplacementsTree;
class StepsBCsTemperaturesTree;
class StepsHistoryOutputsTree;
class StepsFieldOutputsTree;
class StepsManagement;
class CustomLinesTree;
class JobsTree;
class JobsMonitor;
class QTreeWidgetItem;


class NavigationModel;

class CalculiXCoreInterface;
class Claro;
class CommandButtonPanel;

class ModelTree : public QTreeWidget
{
  Q_OBJECT

public:
  ModelTree(QDockWidget *parent);
  ~ModelTree();

  NavigationModel* nav_model;
  CalculiXCoreInterface* ccx_iface;
  Claro* gui;
  CommandButtonPanel* cmdpanel;
  MaterialManagement* myMaterialManagement;
  HBCsManagement* myHBCsManagement;
  StepsManagement* myStepsManagement;
  JobsMonitor* myJobsMonitor;
  std::vector<std::vector<int>> contextMenuAction;

  void setWidgetInCmdPanelMarker(QString name); // set Widget in CommandPanel
  void setWidgetInCmdPanelPath(QString name); // set Widget in CommandPanel
  void execContextMenuAction();
  void update();
  void clear();

public slots:
  void showContextMenu(const QPoint &pos);
  void ContextMenuAction1();
  void ContextMenuAction2();
  void ContextMenuAction3();
  void ContextMenuAction4();
  void ContextMenuAction5();
  void ContextMenuAction6();
  void ContextMenuAction7();
  void ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column);
};

#endif // MODELTREE_HPP
