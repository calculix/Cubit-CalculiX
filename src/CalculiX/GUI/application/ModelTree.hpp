#ifndef MODELTREE_HPP
#define MODELTREE_HPP

#include "GUI/properties/PropertySelection.hpp"
#include "Core/drawing/CoreDrawTypes.hpp"
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
class LoadsSurfaceTractionTree;
class LoadsBodyHeatfluxTree;
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
class StepsLoadsSurfaceTractionTree;
class StepsBodyHeatfluxTree;
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
class Claro;
class TaskPanel;

class ModelTree : public QTreeWidget
{
  Q_OBJECT

public:
  ModelTree(QDockWidget* parent, NavigationModel* navigation);
  ~ModelTree();

  NavigationModel* nav_model;
  Claro* gui;
  TaskPanel* cmdpanel;
  MaterialManagement* myMaterialManagement;
  HBCsManagement* myHBCsManagement;
  StepsManagement* myStepsManagement;
  JobsMonitor* myJobsMonitor;

  void setWidgetInCmdPanelMarker(QString name); // set Widget in CommandPanel
  void setWidgetInCmdPanelPath(QString name); // set Widget in CommandPanel
  void update();
  void clear();

signals:
  void propertySelectionChanged(const PropertySelection& selection);

public slots:
  bool selectDrawingEntity(const CalculiXDrawSelection& selection);
  void updatePropertySelection();
  void showContextMenu(const QPoint &pos);
  void ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column);
};

#endif // MODELTREE_HPP
