
#ifndef CCXDOCKWINDOW_HPP
#define CCXDOCKWINDOW_HPP

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

class CCXDockWindow
{

public:
  CCXDockWindow();
  ~CCXDockWindow();

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
  QSettings *settings;
};

#endif // CCXDOCKWINDOW_HPP
