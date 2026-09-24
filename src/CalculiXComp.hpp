#ifndef CALCULIXCOMP_HPP
#define CALCULIXCOMP_HPP

#include "Component.hpp"
#include "ComponentTracker.hpp"

class MenuManager;
class ToolbarManager;
class cmdPanelManager;
//class ExportManager;
class CCXDockWindowModelTree;
class UserOptionsPanel;
class CalculiXCoreInterface;

class Observer;

class CalculiXComp : public Component, public ComponentTracker
{
public:
  CalculiXComp();
  ~CalculiXComp();

  // Methods inherited from Component
  void start_up(int withGUI);
  void clean_up();
  void clean_up_complete();
  virtual void interrupt_progress();
  void component_loaded(const char* name) override;

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
  //void add_exports();
  //void cleanup_exports();

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
  //ExportManager*  myExportManager;
  CCXDockWindowModelTree* myCCXDockWindowModelTree;
  UserOptionsPanel* mUserOptionsPanel;

  CalculiXCoreInterface *ccx_iface;
  Observer* mListener;
  bool tracker_registered = false;
  bool python_interface_initialized = false;
};

#endif // CALCULIXCOMP_HPP