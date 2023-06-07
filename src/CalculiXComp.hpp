#ifndef CALCULIXCOMP_HPP
#define CALCULIXCOMP_HPP

#include "Component.hpp"

class MenuManager;
class ToolbarManager;
class cmdPanelManager;
class ExportManager;
class CCXDockWindowModelTree;
class UserOptionsPanel;

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

  void setup_CCXDockWindowModelTree();
  void cleanup_CCXDockWindowModelTree();

  //! Add export types to the file menu
  void add_exports();
  void cleanup_exports();

  void setup_observers(int withGUI);
  void cleanup_observers();

  // Add User Options
  void restore_settings();
  void save_settings();
  // load Panel
  void load_options();
  void unload_options();

  MenuManager*    myMenus;
  ToolbarManager* myToolbars;
  cmdPanelManager*   mycmdPanels;
  ExportManager*  myExportManager;
  CCXDockWindowModelTree* myCCXDockWindowModelTree;
  UserOptionsPanel* mUserOptionsPanel;

  Observer* mListener;
};

#endif // CALCULIXCOMP_HPP
