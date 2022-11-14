
#ifndef CCXDOCKWINDOW_HPP
#define CCXDOCKWINDOW_HPP

class Claro;
class ClaroViewMenu;

#include <QObject>
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

private:
  bool isInitialized;
  QString dock_title;
  
  Claro* myClaro;
  ClaroViewMenu* ViewMenu;
  QTreeWidget *myModelTree;
  BlocksTree *myBlocksTree; //Blocks aka Element sets root item
};

#endif // CCXDOCKWINDOW_HPP
