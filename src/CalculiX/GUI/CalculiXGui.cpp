#include "CalculiXGui.hpp"
#include "properties/CalculiXPropertiesDock.hpp"
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/application/CoreObserver.hpp"
#include "CalculiXSettings.hpp"
#include "Claro.hpp"
#include "Broker.hpp"
#include "OptionsPanel.hpp"
#include "CubitMessage.hpp"
#include "MenuManager.hpp"
#include "cmdPanelManager.hpp"
#include "CCXDockWindowModelTree.hpp"
#include "Observer.hpp"
#include "UserOptionsPanel.hpp"

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
  propertiesDock = new CalculiXPropertiesDock(host);
  Claro::instance()->add_dock_window(propertiesDock.data(), "CalculiXProperties", Qt::LeftDockWidgetArea, Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  propertiesDock->hide();
  setup_menus();
  load_options();
  setup_observers(1);
  setup_window_title();
  boolwithGUI = true;

  return true;
}

void CalculiXGui::stop() noexcept
{
  try
  {
    const bool save_options = boolwithGUI;
    boolwithGUI = false;
    cleanup_window_title();
    cleanup_observers();

    if (save_options)
    {
      CalculiXSettings::save();
    }

    unload_options();
    cleanup_menus();
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
      myCCXDockWindowModelTree->update();
    }
  }
}

void CalculiXGui::reset()
{
  if (boolwithGUI)
  {
    if (myCCXDockWindowModelTree)
    {
      myCCXDockWindowModelTree->reset();
    }
  }
}

void CalculiXGui::setup_menus()
{
  if (!myMenus)
  {
    myMenus = new MenuManager;
  }

  myMenus->setup_workspace_menus(Claro::instance(), myCCXDockWindowModelTree->dock_widget(), propertiesDock.data());
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
    delete mListener;
    mListener = nullptr;
  }
}

void CalculiXGui::load_options()
{
  if (mUserOptionsPanel != nullptr)
  {
    return;
  }

  auto* gui = Claro::instance();

  if (gui == nullptr || gui->options() == nullptr)
  {
    return;
  }

  mUserOptionsPanel = new UserOptionsPanel();
  auto* options = gui->options();
  options->add_panel("CalculiX", mUserOptionsPanel.data());
  QObject::connect(options, SIGNAL(refreshSettings()),
    mUserOptionsPanel.data(), SLOT(refresh_settings()));
  QObject::connect(options, SIGNAL(saveSettings()),
    mUserOptionsPanel.data(), SLOT(save_settings()));
}

void CalculiXGui::unload_options()
{
  if (mUserOptionsPanel == nullptr)
  {
    return;
  }

  if (auto* gui = Claro::instance())
  {
    if (gui->options() != nullptr)
    {
      gui->options()->remove_panel("CalculiX");
    }
  }

  delete mUserOptionsPanel.data();
  mUserOptionsPanel = nullptr;
}

bool CalculiXGui::workspace_view(WorkspaceView& view, std::string& error) const
{
  if (!boolwithGUI || mycmdPanels == nullptr || myCCXDockWindowModelTree == nullptr || propertiesDock == nullptr || myMenus == nullptr)
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
  view.label = "CalculiX";
  view.icon = resources.getIcon2("CCX");
  view.docks = {myCCXDockWindowModelTree->dock_widget(), propertiesDock.data()};
  view.menuActions = myMenus->workspace_menu_actions();
  view.navigationModel = model;
  // Cubit uses three button levels, followed by panel-choice dropdowns.
  // Keep CCX as the first level instead of removing it from the path.
  view.navigationRoot = QPersistentModelIndex();
  view.initialNavigationSelection = model->getIndex(root);
  view.hideNavigationMode = true;
  return true;
}
