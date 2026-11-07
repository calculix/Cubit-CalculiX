#include "DocumentationManager.hpp"
#include "CommandDispatcher.hpp"
#include "Claro.hpp"
#include "CubitMessage.hpp"
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStyle>

DocumentationManager::DocumentationManager() = default;

DocumentationManager::~DocumentationManager()
{
  detach();
}

bool DocumentationManager::attach(Claro* window, std::string& error)
{
  error.clear();
  detach();
  if (window == nullptr || window->menuBar() == nullptr)
  {
    error = "Documentation menus require the Cubit GUI.";
    return false;
  }
  host = window;
  for (auto* entry : window->menuBar()->actions())
  {
    QString label = entry->text();
    label.remove('&');
    if (entry->menu() != nullptr && label.compare(tr("Help"), Qt::CaseInsensitive) == 0)
    {
      nativeHelp = entry->menu();
      break;
    }
  }
  frameworkHelp = new QMenu(tr("&Help"), window->menuBar());
  frameworkHelp->setObjectName("Framework.Menu.Help");
  window->menuBar()->addMenu(frameworkHelp.data());
  action = new QAction(window->style()->standardIcon(QStyle::SP_DialogHelpButton), tr("cubit-sim documentation"), this);
  action->setObjectName("Framework.Documentation");
  action->setToolTip(tr("Open the installed offline Framework and workspace documentation."));
  frameworkHelp->addAction(action.data());
  if (nativeHelp != nullptr)
  {
    nativeHelp->addAction(action.data());
  }
  connect(action.data(), &QAction::triggered, this, [this]()
  {
    std::string error;
    if (!CommandDispatcher::submit(host.data(), "sim documentation", error))
    {
      PRINT_ERROR("Unable to submit documentation command: %s\n", error.c_str());
    }
  });
  set_native_workspace(true);
  return true;
}

void DocumentationManager::set_native_workspace(bool native)
{
  if (host == nullptr || host->menuBar() == nullptr || frameworkHelp == nullptr)
  {
    return;
  }

  // Cubit keeps its native Help entries. Application workspaces share the one
  // Framework menu instead of creating their own documentation menus.
  const bool showNative = native && nativeHelp != nullptr && nativeHelp->menuAction()->isVisible();
  frameworkHelp->menuAction()->setVisible(!showNative);

  if (!native && nativeHelp != nullptr)
  {
    nativeHelp->menuAction()->setVisible(false);
  }

  auto* helpAction = showNative ? nativeHelp->menuAction() : frameworkHelp->menuAction();
  auto* menuBar = host->menuBar();

  // Workspace menus may have been appended after attach(). Reorder the active
  // Help entry after visibility is applied on every workspace activation.
  if (menuBar->actions().isEmpty() || menuBar->actions().last() != helpAction)
  {
    menuBar->removeAction(helpAction);
    menuBar->addAction(helpAction);
  }
}

void DocumentationManager::detach() noexcept
{
  if (nativeHelp != nullptr && action != nullptr)
  {
    nativeHelp->removeAction(action.data());
  }
  delete action.data();
  action = nullptr;
  delete frameworkHelp.data();
  frameworkHelp = nullptr;
  nativeHelp = nullptr;
  host = nullptr;
}
