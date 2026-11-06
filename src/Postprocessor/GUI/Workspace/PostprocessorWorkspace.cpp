#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorProperties.hpp"
#include "CubitMessage.hpp"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QStyle>
#include <QToolBar>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>
#include <exception>

PostprocessorWorkspace::PostprocessorWorkspace(QMainWindow* host,
  std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&, std::string&)> submit)
  : QWidget(host), application(std::move(application)), submitCommand(std::move(submit))
{
  setObjectName("Postprocessor.Workspace");
  hide();
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(new QLabel(tr("3D View 1"), this));
  layout->addWidget(new PostprocessorViewport(this), 1);

  resultsDock = new QDockWidget(tr("Sources / Results"), host);
  resultsDock->setObjectName("Postprocessor.ResultsDock");
  tree = new PostprocessorResultsTree([this](const QString& name)
  {
    source_command("select", name);
    // Submission does not imply success; always restore committed selection.
    QTimer::singleShot(0, this, [this]()
    {
      refresh();
    });
  }, resultsDock);
  resultsDock->setWidget(tree);
  resultsDock->hide();

  propertiesDock = new QDockWidget(tr("Properties"), host);
  propertiesDock->setObjectName("Postprocessor.PropertiesDock");
  properties = new PostprocessorProperties(propertiesDock);
  propertiesDock->setWidget(properties);
  propertiesDock->hide();

  toolbar = new QToolBar(tr("Postprocessor"), host);
  toolbar->setObjectName("Postprocessor.Toolbar");
  toolbar->setAllowedAreas(Qt::TopToolBarArea);
  toolbar->setMovable(false);
  toolbar->setFloatable(false);
  toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolbar->hide();

  const QIcon listIcon(":/clarogui/cmdwindow_cl.svg");
  auto* list = new QAction(listIcon.isNull() ? style()->standardIcon(QStyle::SP_FileDialogDetailedView) : listIcon, tr("List sources"), this);
  list->setToolTip(tr("List opened Postprocessor sources in the command window"));
  connect(list, &QAction::triggered, this, [this]()
  {
    this->submit("post list");
  });

  const QIcon closeIcon(":/clarogui/dcancel.svg");
  closeSelected = new QAction(closeIcon.isNull() ? style()->standardIcon(QStyle::SP_DialogCloseButton) : closeIcon, tr("Close selected source"), this);
  closeSelected->setToolTip(tr("Close the selected source without deleting its job or files"));
  connect(closeSelected, &QAction::triggered, this, [this]()
  {
    const auto active = this->application.lock();

    if (active)
    {
      const auto& state = active->session();
      const auto source = state.sources.find(state.active_source_id);

      if (source != state.sources.end())
      {
        source_command("close", QString::fromUtf8(source->second.name.c_str()));
      }
    }
  });

  closeAll = new QAction(tr("Close all sources"), this);
  connect(closeAll, &QAction::triggered, this, [this]()
  {
    this->submit("post close all");
  });

  toolbar->addAction(list);
  toolbar->addAction(closeSelected);

  auto* file = new QMenu(tr("File"), this);
  file->addAction(closeSelected);
  file->addAction(closeAll);
  auto* results = new QMenu(tr("Results"), this);
  results->addAction(list);
  auto* help = new QMenu(tr("Help"), this);
  auto* documentation = help->addAction(tr("Documentation"));
  connect(documentation, &QAction::triggered, this, [this]()
  {
    this->submit("sim documentation");
  });

  for (auto* menu : {file, results, help})
  {
    host->menuBar()->addAction(menu->menuAction());
    menu->menuAction()->setVisible(false);
    menus.append(menu->menuAction());
  }
  refresh();
}

PostprocessorWorkspace::~PostprocessorWorkspace()
{
  delete toolbar;
  delete resultsDock;
  delete propertiesDock;
}

void PostprocessorWorkspace::submit(const QString& command)
{
  std::string error;

  if (!submitCommand || !submitCommand(command.toUtf8().toStdString(), error))
  {
    PRINT_ERROR("Cannot submit Postprocessor command: %s\n", error.c_str());
  }
}

void PostprocessorWorkspace::source_command(const QString& verb, const QString& name)
{
  // Match Cubit's quoted-string syntax without allowing a name to inject a
  // second command. Do not reach into solver-specific GUI quoting utilities.
  const QChar delimiter = name.contains('"') ? QChar(0x27) : QChar('"');

  if (name.isEmpty() || name.contains(delimiter))
  {
    PRINT_ERROR("The source name cannot be represented as one quoted command argument.\n");
    return;
  }

  for (const auto character : name)
  {
    if (character.unicode() < 0x20 || character.unicode() == 0x7f)
    {
      PRINT_ERROR("A source command requires a name without control characters.\n");
      return;
    }
  }

  submit("post " + verb + " " + delimiter + name + delimiter);
}

void PostprocessorWorkspace::refresh()
{
  try
  {
    const auto active = application.lock();

    if (!active)
    {
      return;
    }

    const auto& state = active->session();
    tree->refresh(state);
    const auto source = state.sources.find(state.active_source_id);
    properties->show_source(source == state.sources.end() ? nullptr : &source->second);
    closeSelected->setEnabled(source != state.sources.end());
    closeAll->setEnabled(!state.sources.empty());
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot refresh Postprocessor workspace: %s\n", exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot refresh Postprocessor workspace: unknown failure.\n");
  }
}

void PostprocessorWorkspace::workspace_view(WorkspaceView& view)
{
  view.id = "postprocessor";
  view.label = "Postprocessor";
  view.icon = style()->standardIcon(QStyle::SP_ComputerIcon);
  view.graphicsPage = this;
  view.useNativeNavigation = false;
  view.dockLayout = WorkspaceDockLayout::LeftRight;
  view.docks = {resultsDock, propertiesDock};
  view.menuActions = menus;
  view.toolbars = {toolbar};
  view.activationChanged = [page = QPointer<PostprocessorWorkspace>(this)](bool active)
  {
    if (active && page != nullptr)
    {
      page->refresh();
    }
  };
}
