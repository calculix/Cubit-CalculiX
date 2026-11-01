#ifndef CCXDOCKWINDOWMODELTREE_HPP
#define CCXDOCKWINDOWMODELTREE_HPP

class Claro;
class NavigationModel;
class ClaroViewMenu;

#include <QObject>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
class ModelTree;
class Highlight;
class BlocksTree;
class NodesetTree;
class SidesetTree;
class MaterialTree;
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
class StepsTree;
class TopologyTree;
class CustomLinesTree;
class JobsTree;

class CCXDockWindowModelTree
{

public:
  CCXDockWindowModelTree();
  ~CCXDockWindowModelTree();

  void initialize(NavigationModel* navigation);
  QDockWidget* dock_widget() const noexcept;

  // Cleanup (for example, when we unload the component)
  void clear();

  void update();
  
  void reset();

private:
  bool isInitialized;
  QString dock_title;

  Claro* gui = nullptr;
  ClaroViewMenu* ViewMenu = nullptr;
  QDockWidget *dock = nullptr;
  ModelTree *myModelTree = nullptr;
  Highlight *myHighlight = nullptr;
  BlocksTree *myBlocksTree = nullptr; //Blocks aka Element sets root item
  NodesetTree *myNodesetTree = nullptr; //Node sets root item
  SidesetTree *mySidesetTree = nullptr; //Side sets root item
  MaterialTree *myMaterialTree = nullptr; //Materials root item
  SectionsTree *mySectionsTree = nullptr; //Sections root item
  ConstraintsTree *myConstraintsTree = nullptr; //Constraints root item
  SurfaceInteractionsTree *mySurfaceInteractionsTree = nullptr; //SurfaceInteractions root item
  ContactPairsTree *myContactPairsTree = nullptr; //ContactPairs root item
  AmplitudesTree *myAmplitudesTree = nullptr; //Amplitudes root item
  OrientationsTree *myOrientationsTree = nullptr; //Orientations root item
  DampingTree *myDampingTree = nullptr; //Damping root item
  PhysicalConstantsTree *myPhysicalConstantsTree = nullptr; //PhysicalConstants root item
  LoadsTree *myLoadsTree = nullptr; //Loads root item
  LoadsForcesTree *myLoadsForcesTree = nullptr; //Forces root item
  LoadsPressuresTree *myLoadsPressuresTree = nullptr; //Pressures root item
  LoadsHeatfluxesTree *myLoadsHeatfluxesTree = nullptr; //Heatfluxes root item
  LoadsGravityTree *myLoadsGravityTree = nullptr; //Gravity root item
  LoadsCentrifugalTree *myLoadsCentrifugalTree = nullptr; //Centrifugal root item
  LoadsTrajectoryTree *myLoadsTrajectoryTree = nullptr; //Trajectory root item
  LoadsFilmTree *myLoadsFilmTree = nullptr; //Film root item
  LoadsRadiationTree *myLoadsRadiationTree = nullptr; //Radiation root item
  LoadsSurfaceTractionTree *myLoadsSurfaceTractionTree = nullptr; //Surface Traction root item
  LoadsBodyHeatfluxTree *myLoadsBodyHeatfluxTree = nullptr; //Body Heatflux root item
  BCsTree *myBCsTree = nullptr; //BCs root item
  BCsDisplacementsTree *myBCsDisplacementsTree = nullptr; //Displacements root item
  BCsTemperaturesTree *myBCsTemperaturesTree = nullptr; //Temperature root item
  HistoryOutputsTree *myHistoryOutputsTree = nullptr; //HistoryOutputs root item
  FieldOutputsTree *myFieldOutputsTree = nullptr; //HistoryOutputs root item
  InitialConditionsTree *myInitialConditionsTree = nullptr; //InitialConditions root item
  HBCsTree *myHBCsTree = nullptr; //BCs root item
  HBCsDisplacementsTree *myHBCsDisplacementsTree = nullptr; //Displacements root item
  HBCsTemperaturesTree *myHBCsTemperaturesTree = nullptr; //Temperature root item
  StepsTree *myStepsTree = nullptr; //Steps root item
  TopologyTree *myTopologyTree = nullptr;
  CustomLinesTree *myCustomLinesTree = nullptr; //CustomLines root item
  JobsTree *myJobsTree = nullptr; //Jobs root item
  
};

#endif // CCXDOCKWINDOWMODELTREE
