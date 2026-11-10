#include "PostprocessorWorkspace.hpp"
#include "PostprocessorToolbar.hpp"
#include "Postprocessor/GUI/Sources/PostprocessorSourceControls.hpp"
#include "Postprocessor/GUI/Sources/PostprocessorImportDialog.hpp"
#include <QMessageBox>
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorDeformationControls.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPickControls.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorProperties.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorColorControls.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include "Postprocessor/GUI/Options/PostprocessorOptionsPanel.hpp"
#include "Postprocessor/GUI/Options/PostprocessorMouseOptionsPanel.hpp"
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
#include <QStackedWidget>
#include <QFileDialog>
#include "Postprocessor/GUI/Tables/PostprocessorTableView.hpp"
#include <QToolBar>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>
#include <exception>
#include <algorithm>

PostprocessorWorkspace::PostprocessorWorkspace(QMainWindow* host,
  std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&, std::string&)> submit,
  std::function<bool(const std::string&, std::string&)> silent)
  : QWidget(host), application(std::move(application)), submitCommand(std::move(submit))
{
  setObjectName("Postprocessor.Workspace");
  hide();
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  viewTitle = new QLabel(tr("3D View 1"),this);
  layout->addWidget(viewTitle);
  const auto activeApplication = this->application.lock();
  viewport = new PostprocessorViewport(activeApplication ? activeApplication->session().active_view_id : std::string(),
    this->application,
    [dispatch = submitCommand](const QString& command, std::string& error)
    {
      return dispatch && dispatch(command.toUtf8().toStdString(), error);
    },
    [dispatch = silent](const QString& command, std::string& error)
    {
      return dispatch && dispatch(command.toUtf8().toStdString(), error);
    }, this);
  viewStack = new QStackedWidget(this);
  viewStack->addWidget(viewport);
  tableView = new PostprocessorTableView(viewStack);
  viewStack->addWidget(tableView);
  layout->addWidget(viewStack,1);
  navigationToolbar = new QToolBar(tr("Result navigation"),host);
  navigationToolbar->setObjectName("Postprocessor.NavigationToolbar");
  navigationToolbar->setAllowedAreas(Qt::TopToolBarArea);
  navigationToolbar->setMovable(true);
  navigationToolbar->setFloatable(true);
  navigationToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  navigation = new PostprocessorFrameNavigation([this,dispatch = silent](std::size_t frame)
  {
    const auto command = frame_command(frame);
    if (!command.isEmpty())
    {
      const auto active = this->application.lock();
      if (active && active->user_options().journalNavigation)
      {
        this->submit(command);
      }
      else if (active)
      {
        viewport->cancel_interaction();
        std::string error;
        if (!dispatch || !dispatch(command.toUtf8().toStdString(),error))
        {
          PRINT_ERROR("Cannot select result frame: %s\n",error.c_str());
        }
      }
    }
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  }, navigationToolbar, [this,dispatch = silent](std::size_t frame)
  {
    const auto command = frame_command(frame);
    std::string error;
    const bool accepted = !command.isEmpty() && dispatch && dispatch(command.toUtf8().toStdString(),error);
    if (!accepted && !error.empty())
    {
      PRINT_ERROR("Cannot preview result frame: %s\n",error.c_str());
    }
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
    return accepted;
  },[this,dispatch = silent](const QString& operation)
  {
    // Reuse the source/view/snapshot guard and safe quoting of frame commands.
    const auto guarded = frame_command(1);
    const auto suffix = guarded.indexOf(" source ");
    const auto active = this->application.lock();
    if (active && suffix >= 0)
    {
      const auto command = "post animation " + operation + guarded.mid(suffix);
      const bool navigationAction = operation == "play" || operation == "pause";
      if (!navigationAction || active->user_options().journalNavigation)
      {
        this->submit(command);
      }
      else
      {
        std::string error;
        if (!dispatch || !dispatch(command.toUtf8().toStdString(),error))
        {
          PRINT_ERROR("Cannot change animation: %s\n",error.c_str());
        }
      }
    }
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  });
  navigationToolbar->addWidget(navigation);
  deformationToolbar = new QToolBar(tr("Deformation"),host);
  deformationToolbar->setObjectName("Postprocessor.DeformationToolbar");
  deformationToolbar->setAllowedAreas(Qt::TopToolBarArea);
  deformationToolbar->setMovable(true);
  deformationToolbar->setFloatable(true);
  deformationControls = new PostprocessorDeformationControls(this->application,
    [this](const std::string& command,std::string& error)
    {
      viewport->cancel_interaction();
      const bool accepted = submitCommand && submitCommand(command,error);
      if (!accepted)
      {
        PRINT_ERROR("Cannot change deformation: %s\n",error.c_str());
      }
      QTimer::singleShot(0,this,[this]()
      {
        panelRevision.reset();
        refresh();
      });
      return accepted;
    },[this,dispatch = silent](const std::string& command,std::string& error)
    {
      viewport->cancel_interaction();
      const bool accepted = dispatch && dispatch(command,error);
      if (!accepted)
      {
        PRINT_ERROR("Cannot preview deformation: %s\n",error.c_str());
      }
      QTimer::singleShot(0,this,[this]()
      {
        panelRevision.reset();
        refresh();
      });
      return accepted;
    },deformationToolbar);
  deformationToolbar->addWidget(deformationControls);
  deformationToolbar->hide();
  navigationToolbar->hide();
  optionsPanel = new PostprocessorOptionsPanel(this->application, submitCommand, this);
  optionsPanel->hide();
  mouseOptionsPanel = new PostprocessorMouseOptionsPanel(this->application, submitCommand, this);
  mouseOptionsPanel->hide();

  resultsDock = new QDockWidget(tr("Sources / Results"), host);
  resultsDock->setObjectName("Postprocessor.ResultsDock");
  tree = new PostprocessorResultsTree([this](const QString& name)
  {
    source_command("select", name);
    // Submission does not imply success; always restore committed selection.
    QTimer::singleShot(0, this, [this]()
    {
      panelRevision.reset();
      refresh();
    });
  }, [this](const QString& name, bool visible)
  {
    source_command(visible ? "show" : "hide", name);
    QTimer::singleShot(0, this, [this]()
    {
      panelRevision.reset();
      refresh();
    });
  }, resultsDock);
  tree->open_table = [this](const QString& source,const QString& key,bool field)
  {
    const auto quote = [](const QString& value) -> QString
    {
      const QChar delimiter = value.contains('"') ? QChar(0x27) : QChar('"');
      if (value.contains(delimiter))
      {
        return {};
      }
      for (QChar character : value)
      {
        if (character.unicode() < 32 || character.unicode() == 127)
        {
          return {};
        }
      }
      return delimiter+value+delimiter;
    };
    const auto table = quote(key), name = quote(source);
    if (table.isEmpty() || name.isEmpty())
    {
      PRINT_ERROR("Table/source names cannot be represented by one quoted argument.\n");
      return;
    }
    this->submit(QString(field ? "post table field " : "post table open ")+table+" source "+name);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  };
  tree->category_action = [this](const QString& name,Postprocessor::DatasetCategory category,bool visible)
  {
    const QChar delimiter = name.contains('"') ? QChar(0x27) : QChar('"');
    if (name.contains(delimiter) || name.contains('\n') || name.contains('\r'))
    {
      PRINT_ERROR("Cannot quote the result source name.\n");
      return;
    }
    this->submit(QString(visible ? "post show " : "post hide ")+
      QString::fromLatin1(Postprocessor::dataset_category_name(category))+" source "+delimiter+name+delimiter);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  };
  tree->dataset_action = [this](const QString& source,Postprocessor::DatasetVisibilityAction action,std::optional<std::size_t> dataset)
  {
    dataset_command(source,action,dataset);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  };
  tree->activate_field = [this](const QString& source, std::size_t index)
  {
    QTimer::singleShot(0,this,[this,source,index]()
    {
      colorControls->apply_field(source,index);
    });
  };
  resultsDock->setWidget(tree);
  resultsDock->hide();

  propertiesDock = new QDockWidget(tr("Properties"), host);
  propertiesDock->setObjectName("Postprocessor.PropertiesDock");
  properties = new PostprocessorProperties(this->application,[this](const std::string& command,std::string& error)
  {
    viewport->cancel_interaction();
    const bool accepted = submitCommand && submitCommand(command,error);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
    return accepted;
  },propertiesDock);
  propertiesDock->setWidget(properties);
  propertiesDock->hide();

  nativeToolbar = new PostprocessorToolbar(host);
  nativeToolbar->hide();
  sourceToolbar = new QToolBar(tr("Result sources"),host);
  sourceToolbar->setObjectName("Postprocessor.SourceToolbar");
  sourceToolbar->setAllowedAreas(Qt::TopToolBarArea);
  sourceToolbar->setMovable(true);
  sourceToolbar->setFloatable(true);
  const auto catalogue = [this](std::vector<SimResults::ResultJobEntry>& entries,std::string& error)
  {
    const auto active = this->application.lock();
    return active && active->available_jobs(entries,error);
  };
  sourceControls = new PostprocessorSourceControls(catalogue,[this](const SimResults::ResultJobEntry& job)
  {
    add_job(job);
  },[this]()
  {
    import_results();
  },sourceToolbar);
  sourceToolbar->addWidget(sourceControls);
  sourceToolbar->hide();
  tree->available_jobs = catalogue;
  tree->add_job = [this](const SimResults::ResultJobEntry& job)
  {
    add_job(job);
  };
  tree->import_files = [this]()
  {
    import_results();
  };
  tree->source_action = [this](const QString& source,SourceAction action)
  {
    if (action == SourceAction::RemoveAll)
    {
      if (QMessageBox::question(this,tr("Remove sources"),tr("Remove all opened Postprocessor sources? Jobs and files are retained.")) == QMessageBox::Yes)
      {
        this->submit("post close all");
      }
      return;
    }
    const QString verb = action == SourceAction::Show ? "show" : action == SourceAction::Hide ? "hide" :
      action == SourceAction::ShowOnly ? "show only" : action == SourceAction::Refresh ? "refresh" : "close";
    source_command(verb,source);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  };
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

  const QIcon fitIcon(":/cubitgui/zoomreset.svg");
  auto* fit = new QAction(fitIcon.isNull() ? style()->standardIcon(QStyle::SP_TitleBarMaxButton) : fitIcon, tr("Fit visible results"), this);
  fit->setToolTip(tr("Fit all visible meshes in the active Postprocessor view"));
  connect(fit, &QAction::triggered, this, [this]()
  {
    viewport->fit_view();
  });
  toolbar->addAction(fit);
  pickControls = new PostprocessorPickControls(this->application,[this](const QString& command)
  {
    this->submit(command);
    QTimer::singleShot(0,this,[this]()
    {
      panelSelectionRevision.reset();
      refresh();
    });
  },toolbar);
  toolbar->addWidget(pickControls);
  fieldToolbar = new QToolBar(tr("Field selection"),host);
  fieldToolbar->setObjectName("Postprocessor.FieldToolbar");
  fieldToolbar->setAllowedAreas(Qt::TopToolBarArea);
  fieldToolbar->setMovable(true);
  fieldToolbar->setFloatable(true);
  colorControls = new PostprocessorColorControls(this->application,[this](const QString& command)
  {
    this->submit(command);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  },fieldToolbar);
  fieldToolbar->addWidget(colorControls);
  fieldToolbar->hide();

  auto* file = new QMenu(tr("File"), this);
  file->addAction(closeSelected);
  file->addAction(closeAll);
  auto* importResults = file->addAction(tr("Import result files…"));
  connect(importResults,&QAction::triggered,this,[this]()
  {
    import_results();
  });
  auto* results = new QMenu(tr("Results"), this);
  results->addAction(list);
  auto* probe = results->addAction(tr("Probe selection"));
  connect(probe,&QAction::triggered,this,[this]()
  {
    this->submit("post probe");
  });
  auto* view = new QMenu(tr("View"), this);
  view->addAction(fit);
  auto* meshView = view->addAction(tr("3D view"));
  connect(meshView,&QAction::triggered,this,[this]()
  {
    this->submit("post view type 3d");
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  });
  view->addAction(sourceToolbar->toggleViewAction());
  view->addAction(fieldToolbar->toggleViewAction());
  view->addAction(deformationToolbar->toggleViewAction());
  view->addAction(navigationToolbar->toggleViewAction());

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
  delete nativeToolbar;
  delete sourceToolbar;
  delete toolbar;
  delete resultsDock;
  delete navigationToolbar;
  delete fieldToolbar;
  delete deformationToolbar;
  delete propertiesDock;
  // The options dialog may have reparented this page into its panel stack.
  delete optionsPanel.data();
  delete mouseOptionsPanel.data();
}

void PostprocessorWorkspace::submit(const QString& command)
{
  viewport->cancel_interaction();
  std::string error;

  if (!submitCommand || !submitCommand(command.toUtf8().toStdString(), error))
  {
    PRINT_ERROR("Cannot submit Postprocessor command: %s\n", error.c_str());
  }
}

QString PostprocessorWorkspace::frame_command(std::size_t frame) const
{
  const auto active = application.lock();
  if (!active)
  {
    return {};
  }
  const auto& state = active->session();
  const auto source = state.sources.find(state.active_source_id);
  std::optional<std::size_t> current;
  for (const auto& entry : state.representations)
  {
    const auto& display = entry.second;
    if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.object_id == state.active_source_id &&
        display.input.port == 0 && display.view_id == state.active_view_id)
    {
      current = display.increment_index;
      break;
    }
  }
  const auto view = state.views.find(state.active_view_id);
  if (view != state.views.end() && view->second.table && view->second.table->source_id == state.active_source_id)
  {
    current = view->second.table->frame;
  }
  if (source == state.sources.end() || !navigation->matches_context(state.active_source_id+":"+state.active_view_id,
      active->result_data(state.active_source_id),current))
  {
    return {};
  }
  const auto name = QString::fromUtf8(source->second.name.c_str());
  const QChar delimiter = name.contains('"') ? QChar(0x27) : QChar('"');
  if (name.isEmpty() || name.contains(delimiter))
  {
    return {};
  }
  for (QChar c : name)
  {
    if (c.unicode() < 32 || c.unicode() == 127)
    {
      return {};
    }
  }
  return "post frame " + QString::number(static_cast<qulonglong>(frame)) + " source " + delimiter + name + delimiter;
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

void PostprocessorWorkspace::dataset_command(const QString& name, Postprocessor::DatasetVisibilityAction action,
  std::optional<std::size_t> dataset)
{
  const auto quote = [](const QString& text) -> QString
  {
    const QChar delimiter = text.contains('"') ? QChar(0x27) : QChar('"');
    if (text.isEmpty() || text.contains(delimiter))
    {
      return {};
    }
    for (const auto character : text)
    {
      if (character.unicode() < 32 || character.unicode() == 127)
      {
        return {};
      }
    }
    return delimiter+text+delimiter;
  };
  const auto source = quote(name);
  const auto active = application.lock();
  if (!active || source.isEmpty())
  {
    PRINT_ERROR("Dataset visibility requires a valid source name.\n");
    return;
  }
  std::string sourceId;
  for (const auto& entry : active->session().sources)
  {
    if (QString::fromUtf8(entry.second.name.c_str()) == name)
    {
      sourceId = entry.first;
      break;
    }
  }
  using Action = Postprocessor::DatasetVisibilityAction;
  QString selector = "all";
  if (action != Action::ShowAll)
  {
    if (!dataset)
    {
      return;
    }
    const auto catalogue = active->mesh_datasets(sourceId);
    const auto found = std::find_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value)
    {
      return value.id == *dataset;
    });
    if (found == catalogue.end())
    {
      PRINT_ERROR("The selected mesh dataset is no longer available.\n");
      return;
    }
    selector = QString::number(static_cast<qulonglong>(*dataset));
    const auto label = quote(QString::fromUtf8(found->name.c_str()));
    if (!label.isEmpty() && std::count_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value)
        {
          return value.name == found->name;
        }) == 1)
    {
      selector = label;
    }
  }
  const QString operation = action == Action::Hide ? "hide dataset " :
    action == Action::ShowOnly ? "show only dataset " : "show dataset ";
  submit("post "+operation+selector+" source "+source);
}

bool PostprocessorWorkspace::camera_command_active() const noexcept
{
  return viewport != nullptr && viewport->camera_command_active();
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
    const auto currentView = state.views.find(state.active_view_id);
    const auto selectionRevision = currentView == state.views.end() ? 0 : currentView->second.selection.revision;
    if (!panelSelectionRevision || *panelSelectionRevision != selectionRevision ||
        !panelRevision || *panelRevision != active->display_revision() || panelSource != state.active_source_id || panelView != state.active_view_id)
    {
      tree->refresh(state, [active](const std::string& id)
      {
        return active->result_data(id);
      }, [active](const std::string& id, std::size_t index)
      {
        return active->frame_has_mesh(id, index);
      },[active](const std::string& id,std::optional<std::size_t> frame)
      {
        return active->field_groups(id,frame);
      },[active](const std::string& id)
      {
        return active->mesh_datasets(id);
      },[active](const std::string& id)
      {
        return active->tables(id);
      },[active](const std::string& id)
      {
        return active->source_unavailable_reason(id);
      });
      const bool table = currentView != state.views.end() && currentView->second.table.has_value();
      if (table)
      {
        viewport->cancel_interaction();
        tableView->show_data(active->table_data());
      }
      else
      {
        tableView->show_data({});
      }
      viewStack->setCurrentWidget(table ? static_cast<QWidget*>(tableView) : static_cast<QWidget*>(viewport));
      viewTitle->setText(table ? tr("Table view") : tr("3D View 1"));
      const auto source = state.sources.find(state.active_source_id);
      properties->show_source(source == state.sources.end() ? nullptr : &source->second);
      closeSelected->setEnabled(source != state.sources.end());
      closeAll->setEnabled(!state.sources.empty());
      colorControls->refresh();
      deformationControls->refresh();
      pickControls->refresh();
      pickControls->setEnabled(!table);
      colorControls->setEnabled(!table);
      deformationControls->setEnabled(!table);
      panelSelectionRevision = selectionRevision;
      panelRevision = active->display_revision();
      panelSource = state.active_source_id;
      panelView = state.active_view_id;
      std::optional<std::size_t> frame;
      Postprocessor::AnimationDefinition animation;
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.object_id == panelSource &&
            display.input.port == 0 && display.view_id == panelView)
        {
          frame = display.increment_index;
          animation = display.animation;
          break;
        }
      }
      if (table && currentView->second.table->source_id == panelSource)
      {
        frame = currentView->second.table->frame;
      }
      navigation->refresh(active->result_data(panelSource), frame, [active, id = panelSource](std::size_t index)
      {
        return active->frame_available(id, index);
      },panelSource+":"+panelView,animation,!table);
    }
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
  view.additionalOptionsPages =
  {
    {QStringLiteral("Postprocessor Mouse"), mouseOptionsPanel.data(),
      [panel = mouseOptionsPanel]()
      {
        if (panel != nullptr)
        {
          panel->refresh_settings();
        }
      },
      [panel = mouseOptionsPanel](std::string& error)
      {
        if (panel == nullptr)
        {
          error = "The Postprocessor Mouse options page is unavailable.";
          return false;
        }
        return panel->save_settings(error);
      }}
  };
  view.graphicsPage = this;
  view.useNativeNavigation = false;
  view.dockLayout = WorkspaceDockLayout::LeftRight;
  view.docks = {resultsDock,propertiesDock};
  view.menuActions = menus;
  view.toolbars = {nativeToolbar,toolbar,sourceToolbar,fieldToolbar,deformationToolbar,navigationToolbar};
  view.toolbarRowBreaks = {sourceToolbar};
  view.activationChanged = [page = QPointer<PostprocessorWorkspace>(this)](bool active)
  {
    if (active && page != nullptr)
    {
      page->sourceControls->refresh();
      page->refresh();
    }
    else if (page != nullptr)
    {
      page->viewport->cancel_interaction();
    }
  };
}

bool PostprocessorWorkspace::bind_native_toolbar(Claro* host, std::string& error)
{
  if (!nativeToolbar->bind_native_actions(host,error))
  {
    return false;
  }
  sourceControls->set_import_icon(nativeToolbar->open_icon());
  return true;
}
void PostprocessorWorkspace::add_job(const SimResults::ResultJobEntry& job)
{
  // Prefix is a registered command token, not user-entered display text.
  if (!job.available || job.reference.job_id <= 0 || job.command_prefix.empty() ||
      !std::all_of(job.command_prefix.begin(),job.command_prefix.end(),[](unsigned char value)
      {
        return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
          (value >= '0' && value <= '9') || value == '_';
      }))
  {
    return;
  }
  submit(QStringLiteral("post open ")+QString::fromStdString(job.command_prefix)+" job "+QString::number(job.reference.job_id));
  QTimer::singleShot(0,this,[this]()
  {
    sourceControls->refresh();
    panelRevision.reset();
    refresh();
  });
}
void PostprocessorWorkspace::import_results()
{
  const auto command = PostprocessorImportDialog::command(this);
  if (!command.isEmpty())
  {
    submit(command);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
    });
  }
}
