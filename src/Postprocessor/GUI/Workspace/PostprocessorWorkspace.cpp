#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorProperties.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include "Postprocessor/GUI/Options/PostprocessorOptionsPanel.hpp"
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
  const auto activeApplication = this->application.lock();
  viewport = new PostprocessorViewport(activeApplication ? activeApplication->session().active_view_id : std::string(), this);
  layout->addWidget(viewport, 1);
  optionsPanel = new PostprocessorOptionsPanel(this->application, submitCommand, this);
  optionsPanel->hide();

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
  }, [this](const QString& name, bool visible)
  {
    source_command(visible ? "show" : "hide", name);
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
  const QIcon fitIcon(":/cubitgui/zoomreset.svg");
  auto* fit = new QAction(fitIcon.isNull() ? style()->standardIcon(QStyle::SP_TitleBarMaxButton) : fitIcon, tr("Fit visible results"), this);
  fit->setToolTip(tr("Fit all visible meshes in the active Postprocessor view"));
  connect(fit, &QAction::triggered, this, [this]()
  {
    this->submit("post view fit");
  });
  toolbar->addAction(fit);

  auto* file = new QMenu(tr("File"), this);
  file->addAction(closeSelected);
  file->addAction(closeAll);
  auto* results = new QMenu(tr("Results"), this);
  results->addAction(list);
  auto* view = new QMenu(tr("View"), this);
  view->addAction(fit);

  // Framework supplies the shared Help menu and keeps it last.
  for (auto* menu : {file, view, results})
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
  // The options dialog may have reparented this page into its panel stack.
  delete optionsPanel.data();
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
    tree->refresh(state, [active](const std::string& id)
    {
      return active->source_has_mesh(id);
    });
    const auto source = state.sources.find(state.active_source_id);
    properties->show_source(source == state.sources.end() ? nullptr : &source->second);
    closeSelected->setEnabled(source != state.sources.end());
    closeAll->setEnabled(!state.sources.empty());
    viewport->synchronize(active);
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
  const auto activeApplication = application.lock();
  view.icon = activeApplication ? PostprocessorIconLoader::load(activeApplication->active_icons_path(), "Postprocessor") : QIcon();
  view.optionsPage = optionsPanel;
  view.refreshOptions = [panel = QPointer<PostprocessorOptionsPanel>(optionsPanel)]()
  {
    if (panel != nullptr)
    {
      panel->refresh_settings();
    }
  };
  view.applyOptions = [panel = QPointer<PostprocessorOptionsPanel>(optionsPanel)](std::string& error)
  {
    if (panel == nullptr)
    {
      error = "The Postprocessor options page is unavailable.";
      return false;
    }
    return panel->save_settings(error);
  };
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
