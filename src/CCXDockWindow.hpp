
#ifndef CCXDOCKWINDOW_HPP
#define CCXDOCKWINDOW_HPP

class Claro;
class ClaroViewMenu;

#include <QObject>
#include <QListWidget>

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
  QListWidget *customerList;
};

#endif // CCXDOCKWINDOW_HPP
