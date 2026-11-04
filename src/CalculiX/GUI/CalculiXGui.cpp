#include "GUI/drawing/CalculiXDrawingToolbar.hpp"
#include "GUI/drawing/CalculiXTopologyToolbar.hpp"
#include "GUI/drawing/NativeDisplayRefreshObserver.hpp"
#include "GUI/drawing/DrawingSelectionController.hpp"
#include "GUI/drawing/DrawingPickController.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include <algorithm>
#include "GUI/application/ModelTree.hpp"
#include <QSignalBlocker>
#include "CalculiXGui.hpp"
#include "properties/CalculiXPropertiesDock.hpp"
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/application/CoreObserver.hpp"
#include "Claro.hpp"
#include "Broker.hpp"
#include "CubitMessage.hpp"
#include "MenuManager.hpp"
#include "cmdPanelManager.hpp"
#include "CCXDockWindowModelTree.hpp"
#include "Observer.hpp"
#include "options/UserOptionsPanel.hpp"

#include <QObject>
#include <QCoreApplication>
#include <exception>

CalculiXGui::~CalculiXGui()
{
  stop();
}

bool CalculiXGui::start(QWidget* host, std::string& error)
{
  error.clear();

  if (boolwithGUI)
  {
    return true;
  }

  if (host == nullptr || host != Claro::instance() || Broker::instance() == nullptr)
  {
    error = "CalculiX GUI requires the active Cubit main window and component registry.";
    return false;
  }

  setup_command_panels();
  setup_CCXDockWindowModelTree();
  drawingToolbar = new CalculiXDrawingToolbar(host);
  topologyToolbar = new CalculiXTopologyToolbar(host);
  displayRefreshObserver = new NativeDisplayRefreshObserver(host);
  displayRefreshObserver->displayChanged = [this]()
  {
    drawingToolbar->synchronize(CalculiXDrawInterface().drawing_state());
    topologyToolbar->cancelPendingPreview();
    topologyToolbar->refresh();
  };
  drawingSelection = new DrawingSelectionController(host);
  QObject::connect(myCCXDockWindowModelTree->modelTree(), &ModelTree::propertySelectionChanged, drawingSelection.data(), &DrawingSelectionController::showSelection);
  propertiesDock = new CalculiXPropertiesDock(host);
  QObject::connect(myCCXDockWindowModelTree->modelTree(), &ModelTree::propertySelectionChanged, propertiesDock.data(), &CalculiXPropertiesDock::showSelection);

  drawingPick = new DrawingPickController(myCCXDockWindowModelTree->modelTree(), Claro::instance()->graphics_widget());
  drawingPick->start();

  Claro::instance()->add_dock_window(propertiesDock.data(), "CalculiXProperties", Qt::LeftDockWidgetArea, Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  propertiesDock->hide();
  setup_menus();
  mUserOptionsPanel = new UserOptionsPanel();
  setup_observers(1);
  setup_window_title();
  boolwithGUI = true;

  return true;
}

void CalculiXGui::stop() noexcept
{
  try
  {
    boolwithGUI = false;
    cleanup_window_title();
    cleanup_observers();
    delete displayRefreshObserver.data();
    displayRefreshObserver = nullptr;
    CalculiXDrawInterface drawing;
    drawing.set_workspace_active(false);
    if (drawingPick != nullptr)
    {
      drawingPick->stop();
      delete drawingPick.data();
      drawingPick = nullptr;
    }
    if (drawingSelection != nullptr)
    {
      drawingSelection->clearSelection();
      delete drawingSelection.data();
      drawingSelection = nullptr;
    }
    drawing.shutdown_drawing();

    delete mUserOptionsPanel.data();
    mUserOptionsPanel = nullptr;
    cleanup_menus();
    if (drawingToolbar != nullptr)
    {
      if (auto* host = Claro::instance())
      {
        host->removeToolBar(drawingToolbar.data());
      }
      delete drawingToolbar.data();
      drawingToolbar = nullptr;
    }
    if (topologyToolbar != nullptr)
    {
      if (auto* host = Claro::instance())
      {
        host->removeToolBar(topologyToolbar.data());
      }
      delete topologyToolbar.data();
      topologyToolbar = nullptr;
    }
    if (propertiesDock != nullptr)
    {
      if (auto* host = Claro::instance())
      {
        host->remove_dock_window("CalculiX.Properties");
      }

      delete propertiesDock.data();
      propertiesDock = nullptr;
    }

    cleanup_CCXDockWindowModelTree();
    cleanup_command_panels();
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("CalculiX GUI cleanup failed: %s\n", error.what());
  }
  catch (...)
  {
    PRINT_ERROR("CalculiX GUI cleanup failed.\n");
  }
}

void CalculiXGui::setup_window_title()
{
  auto* host = Claro::instance();
  CalculiXApplicationInterface application;
  const QString suffix = " -- with Cubit-CalculiX " + QString::fromStdString(application.get_version());

  if (host != nullptr && !host->get_title().contains(suffix))
  {
    titleHost = host;
    titleSuffix = suffix;
    host->set_title(host->get_title() + titleSuffix);
  }
}

void CalculiXGui::cleanup_window_title()
{
  if (!QCoreApplication::closingDown() && titleHost != nullptr && !titleSuffix.isEmpty())
  {
    QString title = titleHost->get_title();
    const auto position = title.lastIndexOf(titleSuffix);

    if (position >= 0)
    {
      title.remove(position, titleSuffix.size());
      titleHost->set_title(title);
    }
  }

  titleHost = nullptr;
  titleSuffix.clear();
}

void CalculiXGui::update()
{
  if (boolwithGUI)
  {
    if (myCCXDockWindowModelTree)
    {
      {
        const QSignalBlocker blocker(myCCXDockWindowModelTree->modelTree());
        myCCXDockWindowModelTree->update();
      }
    }
    if (drawingToolbar != nullptr)
    {
      CalculiXDrawInterface drawing;
      const auto state = drawing.drawing_state();
      const bool enabled = std::any_of(state.groups.begin(), state.groups.end(), [](const auto& group) { return group.second.enabled; });
      if (enabled)
      {
        std::string error;
        if (!drawing.refresh_drawing(error))
        {
          PRINT_ERROR("Drawing refresh failed: %s\n", error.c_str());
        }
      }
      else
      {
        std::string error;
        if (!drawing.restore_pending_drawing(error))
        {
          PRINT_WARNING("Drawing restoration failed: %s\n", error.c_str());
        }
      }
      drawingToolbar->refreshSteps();
      drawingToolbar->synchronize(drawing.drawing_state());
    }
    if (topologyToolbar != nullptr)
    {
      topologyToolbar->refresh();
    }
    if (myCCXDockWindowModelTree)
    {
      myCCXDockWindowModelTree->modelTree()->updatePropertySelection();
    }
    if (drawingPick != nullptr)
    {
      auto* host = Claro::instance();
      drawingPick->refreshCursor(host == nullptr ? nullptr : host->graphics_widget());
    }
  }
}

void CalculiXGui::reset()
{
  if (boolwithGUI)
  {
    if (mycmdPanels != nullptr)
    {
      mycmdPanels->clearTopologyPanels();
    }
    CalculiXDrawInterface drawing;
    if (displayRefreshObserver != nullptr)
    {
      displayRefreshObserver->reset();
    }
    drawing.reset_drawing();
    if (topologyToolbar != nullptr)
    {
      topologyToolbar->clear();
    }
    if (drawingSelection != nullptr)
    {
      drawingSelection->clearSelection();
    }
    if (drawingToolbar != nullptr)
    {
      drawingToolbar->refreshSteps();
      drawingToolbar->synchronize(drawing.drawing_state());
    }
    if (propertiesDock != nullptr)
    {
      propertiesDock->reset();
    }
    if (myCCXDockWindowModelTree)
    {
      myCCXDockWindowModelTree->modelTree()->clearSelection();
      myCCXDockWindowModelTree->reset();
    }
    if (drawingPick != nullptr)
    {
      auto* host = Claro::instance();
      drawingPick->refreshCursor(host == nullptr ? nullptr : host->graphics_widget());
    }
  }
}

void CalculiXGui::setup_menus()
{
  if (!myMenus)
  {
    myMenus = new MenuManager;
  }

  myMenus->setup_workspace_menus(Claro::instance(), myCCXDockWindowModelTree->dock_widget(), propertiesDock.data(), drawingToolbar.data(), topologyToolbar.data());
}

void CalculiXGui::cleanup_menus()
{
  if (myMenus)
  {
    myMenus->remove_menu_items();
    delete myMenus;
    myMenus = nullptr;
  }
}

void CalculiXGui::setup_command_panels()
{
  if (!mycmdPanels)
  {
    mycmdPanels = new cmdPanelManager;
  }

  mycmdPanels->initialize();
}

void CalculiXGui::cleanup_command_panels()
{
  if (mycmdPanels)
  {
    mycmdPanels->clear();
    delete mycmdPanels;
    mycmdPanels = nullptr;
  }
}

void CalculiXGui::setup_CCXDockWindowModelTree()
{
  if (!myCCXDockWindowModelTree)
  {
    myCCXDockWindowModelTree = new CCXDockWindowModelTree;
  }

  myCCXDockWindowModelTree->initialize(mycmdPanels->navigation_model());
}

void CalculiXGui::cleanup_CCXDockWindowModelTree()
{
  if (myCCXDockWindowModelTree)
  {
    myCCXDockWindowModelTree->clear();
    delete myCCXDockWindowModelTree;
    myCCXDockWindowModelTree = nullptr;
  }
}

void CalculiXGui::setup_observers(int withGUI)
{
  CalculiXApplicationInterface ccx_application_iface;

  if (!withGUI || mListener)
  {
    return;
  }

  mListener = new Observer();
  mListener->fetch_comp(this);

  auto& observer = ccx_application_iface.core_observer();
  observer.command_start = [this] { mListener->notify_command_start(); };
  observer.before_core_update = [this] { mListener->before_core_update(); };
  observer.core_updated = [this] { mListener->on_core_updated(); };
  observer.core_reset = [this] { reset(); };
  observer.job_deleted = [this](int id)
  {
    if (myCCXDockWindowModelTree != nullptr)
    {
      myCCXDockWindowModelTree->modelTree()->clearDeletedJob(id);
    }
  };
  observer.topology_data_cleared = [this](int id)
  {
    CalculiXDrawInterface().invalidate_topology(id);
    if (topologyToolbar != nullptr)
    {
      topologyToolbar->invalidateTopology(id);
    }
    if (myCCXDockWindowModelTree != nullptr)
    {
      myCCXDockWindowModelTree->modelTree()->invalidateTopology(id);
    }
    if (mycmdPanels != nullptr)
    {
      mycmdPanels->clearTopologyPanels(id);
    }
  };
}

void CalculiXGui::cleanup_observers()
{
  CalculiXApplicationInterface ccx_application_iface;

  if (mListener)
  {
    auto& observer = ccx_application_iface.core_observer();
    observer.command_start = {};
    observer.before_core_update = {};
    observer.core_updated = {};
    observer.core_reset = {};
    observer.job_deleted = {};
    observer.topology_data_cleared = {};
    delete mListener;
    mListener = nullptr;
  }
}

bool CalculiXGui::workspace_view(WorkspaceView& view, std::string& error) const
{
  if (!boolwithGUI || mycmdPanels == nullptr || myCCXDockWindowModelTree == nullptr || propertiesDock == nullptr || drawingToolbar == nullptr || topologyToolbar == nullptr || myMenus == nullptr)
  {
    error = "The CalculiX workspace widgets are not ready.";
    return false;
  }

  auto* model = mycmdPanels->navigation_model();
  auto* root = model->getMarkedNode("CCX");

  if (root == nullptr)
  {
    error = "CalculiX navigation has no CCX root.";
    return false;
  }

  CalculiXGuiResourcesInterface resources;
  view.id = "calculix";
  view.optionsPage = mUserOptionsPanel.data();
  view.refreshOptions = [panel = mUserOptionsPanel]()
  {
    if (panel != nullptr)
    {
      panel->refresh_settings();
    }
  };
  view.applyOptions = [panel = mUserOptionsPanel](std::string& error)
  {
    if (panel == nullptr)
    {
      error = "The CalculiX options page is unavailable.";
      return false;
    }
    return panel->save_settings(error);
  };
  view.label = "CalculiX";
  view.icon = resources.getIcon2("CCX");
  view.docks = {myCCXDockWindowModelTree->dock_widget(), propertiesDock.data()};
  view.menuActions = myMenus->workspace_menu_actions();
  view.toolbars = {drawingToolbar.data(), topologyToolbar.data()};
  view.activationChanged = [
    toolbar = QPointer<CalculiXDrawingToolbar>(drawingToolbar),
    topology = QPointer<CalculiXTopologyToolbar>(topologyToolbar),
    refresh = QPointer<NativeDisplayRefreshObserver>(displayRefreshObserver),
    pick = QPointer<DrawingPickController>(drawingPick),
    tree = QPointer<ModelTree>(myCCXDockWindowModelTree->modelTree())
  ](bool active)
  {
    if (refresh != nullptr)
    {
      refresh->setActive(active);
    }
    if (topology != nullptr)
    {
      topology->refresh();
    }
    if (toolbar != nullptr)
    {
      CalculiXDrawInterface drawing;
      drawing.set_workspace_active(active);
      toolbar->refreshSteps();
      toolbar->synchronize(drawing.drawing_state());
    }
    if (active && tree != nullptr)
    {
      tree->updatePropertySelection();
    }
    if (pick != nullptr)
    {
      auto* host = Claro::instance();
      pick->refreshCursor(host == nullptr ? nullptr : host->graphics_widget());
    }
  };
  view.navigationModel = model;
  // Cubit uses three button levels, followed by panel-choice dropdowns.
  // Keep CCX as the first level instead of removing it from the path.
  view.navigationRoot = QPersistentModelIndex();
  view.initialNavigationSelection = model->getIndex(root);
  view.hideNavigationMode = true;
  return true;
}
