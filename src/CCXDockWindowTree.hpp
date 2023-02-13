
#ifndef CCXDOCKWINDOWTREE_HPP
#define CCXDOCKWINDOWTREE_HPP

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


class CCXDockWindowTree
{

public:
  CCXDockWindowTree();
  ~CCXDockWindowTree();

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
  QSettings *settings;
};

#endif // CCXDOCKWINDOWTREE_HPP
