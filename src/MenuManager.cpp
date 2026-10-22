/*! @file MenuManager.cpp
 *  @brief Opens the packaged manual through the desktop's local-file handler.
 */
#include "MenuManager.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ComponentInfo.hpp"
#include "Mediator.hpp"
#include "CubitMessage.hpp"

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QUrl>
#include <vector>

namespace
{
const char* documentation_owner = "CalculiXCompDocumentation";
}

MenuManager::MenuManager(QObject* parent) : QObject(parent)
{}

MenuManager::~MenuManager()
{
  remove_menu_items();
}

void MenuManager::setup_help_menu()
{
  if (help_initialized)
  {
    return;
  }

  Claro* gui = Claro::instance();
  if (!gui)
  {
    return;
  }

  help_action = new QAction(tr("Cubit-CalculiX documentation"), this);
  connect(help_action.data(), &QAction::triggered, this, &MenuManager::open_documentation);
  std::vector<QAction*> actions{help_action.data()};
  gui->add_to_menu(tr("&Help"), actions, documentation_owner);
  help_initialized = true;
}

void MenuManager::remove_menu_items()
{
  if (!help_initialized)
  {
    return;
  }

  if (Claro* gui = Claro::instance())
  {
    gui->remove_menu_items(documentation_owner);
  }

  // Claro may delete actions during removal; QPointer tracks that ownership.
  delete help_action.data();
  help_action = nullptr;
  help_initialized = false;
}

void MenuManager::open_documentation()
{
  Mediator* mediator = Broker::instance();
  if (!mediator)
  {
    PRINT_ERROR("Unable to locate Cubit-CalculiX documentation: component registry is unavailable.\n");
    return;
  }

  std::vector<ComponentInfo> components;
  mediator->get_component_list(components);
  QSet<QString> candidates;
  QString component_directory;

  for (auto& component : components)
  {
    const QString path = component.get_path();
    if (path.isEmpty())
    {
      continue;
    }

    const QDir directory(path);
#ifdef WIN32
    const QFileInfo library(directory.filePath("calculix_plugin.dll"));
#else
    const QFileInfo library(directory.filePath("libcalculix_plugin.so"));
#endif
    if (!library.isFile())
    {
      continue;
    }

    const QString resolved = library.canonicalPath();
    if (component.get_class() == "CalculiXComp")
    {
      component_directory = resolved;
      break;
    }
    candidates.insert(resolved);
  }

  if (component_directory.isEmpty() && candidates.size() == 1)
  {
    component_directory = *candidates.constBegin();
  }
  if (component_directory.isEmpty())
  {
    PRINT_ERROR("Unable to locate the Cubit-CalculiX plugin directory. Expected documentation/index.html beside the plugin library.\n");
    return;
  }

  const QString index_path = QDir(component_directory).filePath("documentation/index.html");
  const QFileInfo index(index_path);
  if (!index.isFile() || !index.isReadable())
  {
    PRINT_ERROR("Cubit-CalculiX documentation is missing or unreadable: %s\n", index_path.toUtf8().constData());
    return;
  }

  // Success means the desktop accepted the request, not that rendering completed.
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(index.absoluteFilePath())))
  {
    PRINT_ERROR("Unable to open Cubit-CalculiX documentation: %s\n", index_path.toUtf8().constData());
    return;
  }
}
