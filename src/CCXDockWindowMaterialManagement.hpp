#ifndef CCXDOCKWINDOWMATERIALMANAGEMENT_HPP
#define CCXDOCKWINDOWMATERIALMANAGEMENT_HPP

class Claro;
class ClaroViewMenu;

#include <QObject>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QHeaderView>

class ModelTree;
class BlocksTree;
class NodesetTree;
class SidesetTree;
class MaterialTree;
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
class JobsTree;

class CCXDockWindowMaterialManagement
{

public:
  CCXDockWindowMaterialManagement();
  ~CCXDockWindowMaterialManagement();

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
  BlocksTree *myBlocksTree; //Blocks aka Element sets root item
  NodesetTree *myNodesetTree; //Node sets root item
  SidesetTree *mySidesetTree; //Side sets root item
  MaterialTree *myMaterialTree; //Materials root item
  SectionsTree *mySectionsTree; //Sections root item
  ConstraintsTree *myConstraintsTree; //Constraints root item
  SurfaceInteractionsTree *mySurfaceInteractionsTree; //SurfaceInteractions root item
  ContactPairsTree *myContactPairsTree; //ContactPairs root item
  AmplitudesTree *myAmplitudesTree; //Amplitudes root item
  LoadsTree *myLoadsTree; //Loads root item
  LoadsForcesTree *myLoadsForcesTree; //Forces root item
  LoadsPressuresTree *myLoadsPressuresTree; //Pressures root item
  BCsTree *myBCsTree; //BCs root item
  BCsDisplacementsTree *myBCsDisplacementsTree; //Displacements root item
  BCsTemperaturesTree *myBCsTemperaturesTree; //Temperature root item
  HistoryOutputsTree *myHistoryOutputsTree; //HistoryOutputs root item
  FieldOutputsTree *myFieldOutputsTree; //HistoryOutputs root item
  InitialConditionsTree *myInitialConditionsTree; //InitialConditions root item
  StepsTree *myStepsTree; //Steps root item
  JobsTree *myJobsTree; //Jobs root item
  
  QSettings *settings;
};

#endif // CCXDOCKWINDOWMATERIALMANAGEMENT_HPP