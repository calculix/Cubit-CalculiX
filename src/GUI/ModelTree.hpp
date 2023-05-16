
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
class LoadsTree;
class LoadsForcesTree;
class LoadsPressuresTree;
class BCsTree;
class BCsDisplacementsTree;
class BCsTemperaturesTree;
class HistoryOutputsTree;
class FieldOutputsTree;
class InitialConditionsTree;
class StepsTree;
class StepsLoadsTree;
class StepsLoadsForcesTree;
class StepsLoadsPressuresTree;
class StepsBCsTree;
class StepsBCsDisplacementsTree;
class StepsBCsTemperaturesTree;
class StepsHistoryOutputsTree;
class StepsFieldOutputsTree;
class StepsManagement;
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
  StepsManagement* myStepsManagement;
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
  void ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column);
};

#endif // MODELTREE_HPP
