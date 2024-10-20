#ifndef CCXDOCKWINDOWMODELTREE_HPP
#define CCXDOCKWINDOWMODELTREE_HPP

class Claro;
class ClaroViewMenu;

#include <QObject>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QHeaderView>

class CalculiXCoreInterface;
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
class LoadsTree;
class LoadsForcesTree;
class LoadsPressuresTree;
class LoadsHeatfluxesTree;
class LoadsGravityTree;
class LoadsCentrifugalTree;
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
class CustomLinesTree;
class JobsTree;

class CCXDockWindowModelTree
{

public:
  CCXDockWindowModelTree();
  ~CCXDockWindowModelTree();

  void initialize();

  // Cleanup (for example, when we unload the component)
  void clear();

  void update();
  
  void reset();

private:
  bool isInitialized;
  QString dock_title;

  
  Claro* gui;
  ClaroViewMenu* ViewMenu;
  QDockWidget *dock;
  ModelTree *myModelTree;
  Highlight *myHighlight;
  BlocksTree *myBlocksTree; //Blocks aka Element sets root item
  NodesetTree *myNodesetTree; //Node sets root item
  SidesetTree *mySidesetTree; //Side sets root item
  MaterialTree *myMaterialTree; //Materials root item
  SectionsTree *mySectionsTree; //Sections root item
  ConstraintsTree *myConstraintsTree; //Constraints root item
  SurfaceInteractionsTree *mySurfaceInteractionsTree; //SurfaceInteractions root item
  ContactPairsTree *myContactPairsTree; //ContactPairs root item
  AmplitudesTree *myAmplitudesTree; //Amplitudes root item
  OrientationsTree *myOrientationsTree; //Orientations root item
  LoadsTree *myLoadsTree; //Loads root item
  LoadsForcesTree *myLoadsForcesTree; //Forces root item
  LoadsPressuresTree *myLoadsPressuresTree; //Pressures root item
  LoadsHeatfluxesTree *myLoadsHeatfluxesTree; //Heatfluxes root item
  LoadsGravityTree *myLoadsGravityTree; //Gravity root item
  LoadsCentrifugalTree *myLoadsCentrifugalTree; //Centrifugal root item
  BCsTree *myBCsTree; //BCs root item
  BCsDisplacementsTree *myBCsDisplacementsTree; //Displacements root item
  BCsTemperaturesTree *myBCsTemperaturesTree; //Temperature root item
  HistoryOutputsTree *myHistoryOutputsTree; //HistoryOutputs root item
  FieldOutputsTree *myFieldOutputsTree; //HistoryOutputs root item
  InitialConditionsTree *myInitialConditionsTree; //InitialConditions root item
  HBCsTree *myHBCsTree; //BCs root item
  HBCsDisplacementsTree *myHBCsDisplacementsTree; //Displacements root item
  HBCsTemperaturesTree *myHBCsTemperaturesTree; //Temperature root item
  StepsTree *myStepsTree; //Steps root item
  CustomLinesTree *myCustomLinesTree; //CustomLines root item
  JobsTree *myJobsTree; //Jobs root item
  
  QSettings *settings;
  CalculiXCoreInterface *ccx_iface;
};

#endif // CCXDOCKWINDOWMODELTREE
