/*!
 *  \file MyComp.hpp
 *  \brief
 *    Provides an example of creating a custom component.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#ifndef CALCULIXCOMP_HPP
#define CALCULIXCOMP_HPP

#include "Component.hpp"

class MenuManager;
class ToolbarManager;
class cmdPanelManager;
class ExportManager;
class CCXDockWindow;

class Observer;

class CalculiXComp : public Component
{
public:
  CalculiXComp();
  ~CalculiXComp();

  // Methods inherited from Component
  void start_up(int withGUI);
  void clean_up();

  void update();
  void reset();

  bool boolwithGUI;

private:
  void setup_menus();
  void cleanup_menus();

  void setup_toolbars();
  void cleanup_toolbars();

  void setup_command_panels();
  void cleanup_command_panels();

  void setup_CCXDockWindow();
  void cleanup_CCXDockWindow();

  //! Add export types to the file menu
  void add_exports();
  void cleanup_exports();

  void setup_observers(int withGUI);
  void cleanup_observers();

  MenuManager*    myMenus;
  ToolbarManager* myToolbars;
  cmdPanelManager*   mycmdPanels;
  ExportManager*  myExportManager;
  CCXDockWindow* myCCXDockWindow;

  Observer* mListener;
};

#endif // CALCULIXCOMP_HPP
