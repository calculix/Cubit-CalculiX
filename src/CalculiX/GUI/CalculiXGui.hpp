#ifndef CALCULIXGUI_HPP
#define CALCULIXGUI_HPP

#include <QPointer>
#include <QString>
#include <string>

struct WorkspaceView;
class Claro;
class CalculiXPropertiesDock;
class CalculiXDrawingToolbar;
class CalculiXTopologyToolbar;
class NativeDisplayRefreshObserver;
class DrawingSelectionController;
class DrawingPickController;
class QWidget;
class MenuManager;
class ToolbarManager;
class cmdPanelManager;
class CCXDockWindowModelTree;
class UserOptionsPanel;
class Observer;

// Internal GUI owner. FEAComp is the only loader-facing component.
class CalculiXGui
{
public:
  CalculiXGui() = default;
  ~CalculiXGui();

  bool start(QWidget* host, std::string& error);
  void stop() noexcept;
  void update();
  void reset();
  bool workspace_view(WorkspaceView& view, std::string& error) const;

private:
  void setup_menus();
  void cleanup_menus();
  void setup_command_panels();
  void cleanup_command_panels();
  void setup_CCXDockWindowModelTree();
  void cleanup_CCXDockWindowModelTree();
  void setup_observers(int withGUI);
  void cleanup_observers();
  void setup_window_title();
  void cleanup_window_title();

  MenuManager* myMenus = nullptr;
  cmdPanelManager* mycmdPanels = nullptr;
  CCXDockWindowModelTree* myCCXDockWindowModelTree = nullptr;
  QPointer<UserOptionsPanel> mUserOptionsPanel;
  QPointer<CalculiXPropertiesDock> propertiesDock;
  QPointer<CalculiXDrawingToolbar> drawingToolbar;
  QPointer<CalculiXTopologyToolbar> topologyToolbar;
  QPointer<NativeDisplayRefreshObserver> displayRefreshObserver;
  QPointer<DrawingSelectionController> drawingSelection;
  QPointer<DrawingPickController> drawingPick;
  QPointer<Claro> titleHost;
  QString titleSuffix;
  Observer* mListener = nullptr;
  bool boolwithGUI = false;
};

#endif // CALCULIXGUI_HPP
