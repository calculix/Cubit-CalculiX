
#ifndef CCXDOCKWINDOW_HPP
#define CCXDOCKWINDOW_HPP

class Claro;
class ClaroViewMenu;

#include <QObject>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class BlocksTree;

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

  
  Claro* myClaro;
  ClaroViewMenu* ViewMenu;
  QDockWidget *dock;
  QTreeWidget *myModelTree;
  BlocksTree *myBlocksTree; //Blocks aka Element sets root item
  
};

#endif // CCXDOCKWINDOW_HPP
