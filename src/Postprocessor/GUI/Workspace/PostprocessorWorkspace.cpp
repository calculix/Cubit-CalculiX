#include "Postprocessor/GUI/Properties/PostprocessorDisplayProperties.hpp"
#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/GUI/VectorPlot/PostprocessorVectorLegendProperties.hpp"
#include "Postprocessor/GUI/Explosion/PostprocessorExplosionControls.hpp"
#include "Postprocessor/GUI/Explosion/PostprocessorExplosionPlayback.hpp"
#include "Postprocessor/GUI/VectorPlot/PostprocessorVectorControls.hpp"
#include "Postprocessor/GUI/Filters/Clip/PostprocessorClipControls.hpp"
#include "Postprocessor/GUI/Filters/Section/PostprocessorSectionControls.hpp"
#include "Postprocessor/GUI/Filters/Threshold/PostprocessorThresholdControls.hpp"
#include "Postprocessor/Commands/tables/TableCommandText.hpp"
#include <QProgressDialog>
#include "PostprocessorToolbar.hpp"
#include "Postprocessor/GUI/Sources/PostprocessorSourceControls.hpp"
#include "Postprocessor/GUI/Sources/PostprocessorImportDialog.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorDeformationControls.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPickControls.hpp"
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationControls.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorProperties.hpp"
#include "Postprocessor/Commands/display/AppearanceCommandText.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorColorControls.hpp"
#include "Postprocessor/GUI/Options/PostprocessorOptionsPanel.hpp"
#include "Postprocessor/GUI/Options/PostprocessorMouseOptionsPanel.hpp"
#include "CubitMessage.hpp"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QIcon>
#include <QMainWindow>
#include <QTabWidget>
#include <QSignalBlocker>
#include <QScopedValueRollback>
#include "Postprocessor/GUI/Plots/PostprocessorPlotPane.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotControls.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include <QPointer>
#include <QSplitter>
#include "Postprocessor/GUI/Tables/PostprocessorTableView.hpp"
#include <QToolBar>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <utility>
#include <exception>
#include <algorithm>

PostprocessorWorkspace::PostprocessorWorkspace(QMainWindow* host,
  std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&, std::string&)> submit,
  std::function<bool(const std::string&, std::string&)> silent)
  : QWidget(host), application(std::move(application)), submitCommand(std::move(submit)), silentCommand(silent)
{
  setObjectName("Postprocessor.Workspace");
  hide();
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  viewTitle = new QLabel(tr("3D View 1"),this);
  const auto activeApplication = this->application.lock();
  std::string initialView;
  if (activeApplication != nullptr) {
    initialView = activeApplication->session().active_view_id;
  }
  viewport = new PostprocessorViewport(initialView,
    this->application,
    [dispatch = submitCommand](const QString& command, std::string& error) {
      return dispatch && dispatch(command.toUtf8().toStdString(), error);
    },
    [dispatch = silent](const QString& command, std::string& error) {
      return dispatch && dispatch(command.toUtf8().toStdString(), error);
    }, this);
  viewSplitter = new QSplitter(Qt::Horizontal,this);
  viewSplitter->setObjectName("Postprocessor.ViewTableSplitter");
  viewSplitter->setChildrenCollapsible(false);
  auto* viewportPane = new QWidget(viewSplitter);
  auto* viewportLayout = new QVBoxLayout(viewportPane);
  viewportLayout->setContentsMargins(0,0,0,0);
  viewportLayout->addWidget(viewTitle);
  viewportLayout->addWidget(viewport,1);
  viewportPane->setMinimumWidth(150);
  resultTabs = new QTabWidget(viewSplitter);
  resultTabs->setObjectName("Postprocessor.ResultTabs");
  resultTabs->setMinimumWidth(200);
  plotPane = new PostprocessorPlotPane(this->application,
    [this](const QString& command,std::string& error) {
      const bool accepted = submitCommand && submitCommand(command.toUtf8().toStdString(),error);
      QTimer::singleShot(0,this,[this]() {
        refresh();
      });
      return accepted;
    },[dispatch = silent](const QString& command,std::string& error) {
      return dispatch && dispatch(command.toUtf8().toStdString(),error);
    },resultTabs);
  tableView = new PostprocessorTableView([this]() {
    this->submit("post view type 3d");
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
      viewport->request_redraw();
    });
  },resultTabs,[this](const Postprocessor::TableReference& reference,const Postprocessor::ResultTableData& data,std::size_t row,std::size_t column) {
    const auto active = this->application.lock();
    if (active) {
      plot_table_dialog(this,*active,reference,data,row,column,[this](const QString& command) {
        this->submit(command);
        refresh();
      });
    }
  },[this](const QString& command)
  {
    this->submit(command);
    refresh();
  });
  tableView->setMinimumWidth(150);
  tableView->hide();
  plotPane->hide();
  viewSplitter->addWidget(viewportPane);
  viewSplitter->addWidget(resultTabs);
  viewSplitter->setStretchFactor(0,2);
  viewSplitter->setStretchFactor(1,1);
  resultTabs->hide();
  connect(resultTabs,&QTabWidget::currentChanged,this,[this](int index) {
    const auto active = this->application.lock();
    if (!active || index < 0) {
      return;
    }
    const auto& state = active->session();
    const auto view = state.views.find(state.active_view_id);
    if (view == state.views.end()) {
      return;
    }
    if (resultTabs->widget(index) == plotPane && view->second.plot_id) {
      this->submit(QStringLiteral("post plot show ")+Postprocessor::plot_quote(state.plots.at(*view->second.plot_id).name));
    } else if (resultTabs->widget(index) == tableView && view->second.table) {
      const auto reference = *view->second.table;
      const auto source = state.sources.find(reference.source_id);
      if (source != state.sources.end()) {
        std::string error;
        const auto command = Postprocessor::table_open_command_text(*active,reference,error);
        if (command.isEmpty()) {
          PRINT_ERROR("Cannot journal table selection: %s\n",error.c_str());
        } else {
          this->submit(command);
        }
      }
    }
    refresh();
  });
  layout->addWidget(viewSplitter,1);
  navigationToolbar = new QToolBar(tr("Result navigation"),host);
  navigationToolbar->setObjectName("Postprocessor.NavigationToolbar");
  navigationToolbar->setAllowedAreas(Qt::NoToolBarArea);
  navigationToolbar->setMovable(false);
  navigationToolbar->setFloatable(false);
  navigationToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  navigationToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  navigation = new PostprocessorFrameNavigation([this,dispatch = silent](std::size_t frame) {
    const auto command = frame_command(frame);
    if (!command.isEmpty()) {
      const auto active = this->application.lock();
      if (active && active->user_options().journalNavigation) {
        this->submit(command);
      } else if (active) {
        const QScopedValueRollback<bool> guard(foregroundCommand,true);
        viewport->cancel_interaction();
        std::string error;
        if (!dispatch || !dispatch(command.toUtf8().toStdString(),error)) {
          PRINT_ERROR("Cannot select result frame: %s\n",error.c_str());
        }
      }
    }
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  }, navigationToolbar, [this,dispatch = silent](std::size_t frame) {
    const QScopedValueRollback<bool> guard(foregroundCommand,true);
    const auto command = frame_command(frame);
    std::string error;
    const bool accepted = !command.isEmpty() && dispatch && dispatch(command.toUtf8().toStdString(),error);
    if (!accepted && !error.empty()) {
      PRINT_ERROR("Cannot preview result frame: %s\n",error.c_str());
    }
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
    return accepted;
  },[this,dispatch = silent](const QString& operation) {
    // Reuse the source/view/snapshot guard and safe quoting of frame commands.
    const auto guarded = frame_command(1);
    const auto suffix = guarded.indexOf(" source ");
    const auto active = this->application.lock();
    if (active && suffix >= 0) {
      const auto command = "post animation " + operation + guarded.mid(suffix);
      const bool navigationAction = operation == "play" || operation == "pause";
      if (!navigationAction || active->user_options().journalNavigation) {
        this->submit(command);
      } else {
        std::string error;
        if (!dispatch || !dispatch(command.toUtf8().toStdString(),error)) {
          PRINT_ERROR("Cannot change animation: %s\n",error.c_str());
        }
      }
    }
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  },[this]()
  {
    return viewport->prepare_animation_step();
  },[this,dispatch = silent](double seconds)
  {
    const auto guarded = frame_command(1);
    const auto suffix = guarded.indexOf(" source ");
    const auto active = this->application.lock();
    if (!active || suffix < 0 || !dispatch)
    {
      return false;
    }
    if (!viewport->animation_rotation_ready())
    {
      return true;
    }
    const QScopedValueRollback<bool> guard(animationCameraCommand,true);
    std::string error;
    const auto command = "post animation advance "+QString::number(seconds,'g',17)+guarded.mid(suffix);
    if (!dispatch(command.toUtf8().toStdString(),error))
    {
      PRINT_ERROR("Cannot advance animation camera: %s\n",error.c_str());
      return false;
    }
    viewport->synchronize(active);
    return true;
  });
  if (const auto active = this->application.lock())
  {
    navigation->set_loop_icon(PostprocessorIconLoader::load(active->active_icons_path(),"PostprocessorAnimationLoop"));
    navigation->set_rotation_icons({
      PostprocessorIconLoader::load(active->active_icons_path(),"PostprocessorAnimationRotationX"),
      PostprocessorIconLoader::load(active->active_icons_path(),"PostprocessorAnimationRotationY"),
      PostprocessorIconLoader::load(active->active_icons_path(),"PostprocessorAnimationRotationZ")});
  }
  navigationToolbar->addWidget(navigation);
  navigationDock = new QDockWidget(tr("Result navigation"), host);
  navigationDock->setObjectName("Postprocessor.NavigationDock");
  navigationDock->setAllowedAreas(Qt::BottomDockWidgetArea);
  navigationDock->setFeatures(QDockWidget::DockWidgetClosable);
  navigationDock->setTitleBarWidget(new QWidget(navigationDock));
  navigationDock->hide();
  deformationToolbar = new QToolBar(tr("Deformation"),host);
  // A new name prevents old main-window toolbar state from restoring this
  // embedded toolbar into the top area.
  deformationToolbar->setObjectName("Postprocessor.NavigationDeformationToolbar");
  deformationToolbar->setAllowedAreas(Qt::NoToolBarArea);
  deformationToolbar->setMovable(false);
  deformationToolbar->setFloatable(false);
  deformationControls = new PostprocessorDeformationControls(this->application,
    [this](const std::string& command,std::string& error) {
      viewport->cancel_interaction();
      const bool accepted = submitCommand && submitCommand(command,error);
      if (!accepted) {
        PRINT_ERROR("Cannot change deformation: %s\n",error.c_str());
      }
      QTimer::singleShot(0,this,[this]() {
        panelRevision.reset();
        refresh();
      });
      return accepted;
    },[this,dispatch = silent](const std::string& command,std::string& error) {
      viewport->cancel_interaction();
      const bool accepted = dispatch && dispatch(command,error);
      if (!accepted) {
        PRINT_ERROR("Cannot preview deformation: %s\n",error.c_str());
      }
      QTimer::singleShot(0,this,[this]() {
        panelRevision.reset();
        refresh();
      });
      return accepted;
    },deformationToolbar);
  deformationToolbar->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);
  deformationToolbar->addWidget(deformationControls);
  vectorControls = new PostprocessorVectorControls(this->application,
    [this](const std::string& command,std::string& error)
    {
      viewport->cancel_interaction();
      const bool accepted = submitCommand && submitCommand(command,error);
      QTimer::singleShot(0,this,[this]()
      {
        panelRevision.reset();
        refresh();
      });
      return accepted;
    },this);
  vectorControls->hide();

  auto* navigationRow = new QWidget(navigationDock);
  navigationRow->setObjectName("Postprocessor.NavigationRow");
  navigationRow->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  auto* navigationLayout = new QHBoxLayout(navigationRow);
  navigationLayout->setContentsMargins(0,0,0,0);
  navigationLayout->setSpacing(4);
  navigationLayout->addWidget(navigationToolbar,1);
  navigationLayout->addWidget(deformationToolbar,0);
  navigationDock->setWidget(navigationRow);
  deformationToolbar->show();
  navigationToolbar->show();
  optionsPanel = new PostprocessorOptionsPanel(this->application, submitCommand, this);
  optionsPanel->hide();
  mouseOptionsPanel = new PostprocessorMouseOptionsPanel(this->application, submitCommand, this);
  mouseOptionsPanel->hide();

  resultsDock = new QDockWidget(tr("Sources / Results"), host);
  resultsDock->setObjectName("Postprocessor.ResultsDock");
  tree = new PostprocessorResultsTree([this](const QString& name) {
    source_command("select", name);
    // Submission does not imply success; always restore committed selection.
    QTimer::singleShot(0, this, [this]() {
      panelRevision.reset();
      refresh();
    });
  }, [this](const QString& name, bool visible) {
    QString operation = "hide";
    if (visible) {
      operation = "show";
    }
    source_command(operation, name);
    QTimer::singleShot(0, this, [this]() {
      panelRevision.reset();
      refresh();
    });
  }, resultsDock);
  if (const auto active = this->application.lock())
  {
    tree->set_icons_directory(active->active_icons_path());
  }
  tree->filter_action = [this](const QString& name,const QString& action,const QString& replacement)
  {
    const auto quoted = Postprocessor::plot_quote(name.toUtf8().toStdString());
    if (quoted.isEmpty())
    {
      return;
    }
    QString command = "post filter "+action+" "+quoted;
    if (action == "rename")
    {
      const auto value = Postprocessor::plot_quote(replacement.toUtf8().toStdString());
      if (value.isEmpty())
      {
        return;
      }
      command += " "+value;
    }
    this->submit(command);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
      viewport->request_redraw();
    });
  };
  tree->inspect_source = [this](const QString& name)
  {
    const auto quoted = Postprocessor::plot_quote(name.toUtf8().toStdString());
    if (!quoted.isEmpty())
    {
      this->submit("post properties source "+quoted);
    }
  };
  tree->selection_cleared = [this]()
  {
    if (propertiesPage == "source")
    {
      this->submit("post properties close");
    }
  };
  tree->open_table = [this](const QString& source,const QString& key,bool field) {
    const auto active = this->application.lock();
    if (!active) {
      return;
    }
    std::string sourceId;
    for (const auto& entry : active->session().sources) {
      if (QString::fromUtf8(entry.second.name.c_str()) == source) {
        sourceId = entry.first;
        break;
      }
    }
    std::string error;
    const Postprocessor::TableReference reference{sourceId,key.toUtf8().toStdString(),field,0};
    const auto command = Postprocessor::table_open_command_text(*active,reference,error);
    if (command.isEmpty()) {
      PRINT_ERROR("Cannot journal table selection: %s\n",error.c_str());
      return;
    }
    this->submit(command);
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  };
  tree->category_action = [this](const QString& name,Postprocessor::DatasetCategory category,bool visible) {
    QChar delimiter('"');
    if (name.contains(delimiter)) {
      delimiter = QChar(0x27);
    }
    if (name.contains(delimiter) || name.contains('\n') || name.contains('\r')) {
      PRINT_ERROR("Cannot quote the result source name.\n");
      return;
    }
    QString operation = "post hide ";
    if (visible) {
      operation = "post show ";
    }
    this->submit(operation + QString::fromLatin1(Postprocessor::dataset_category_name(category)) + " source " + delimiter + name + delimiter);
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  };
  tree->dataset_action = [this](const QString& source,Postprocessor::DatasetVisibilityAction action,std::optional<std::size_t> dataset) {
    dataset_command(source,action,dataset);
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  };
  tree->activate_field = [this](const QString& source, std::size_t index) {
    QTimer::singleShot(0,this,[this,source,index]() {
      colorControls->apply_field(source,index);
    });
  };
  tree->activate_component = [this](const QString& source, std::size_t index, const QString& component, bool magnitude)
  {
    colorControls->apply_field(source,index,component.toUtf8().toStdString(),magnitude);
  };
  resultsDock->setWidget(tree);
  resultsDock->hide();

  create_properties_dock(host);
  tree->appearance_value = [this](const Postprocessor::AppearanceTarget& target)
  {
    if (const auto app = this->application.lock())
    {
      return app->dataset_appearance(target.source_id,target.view_id,target.dataset_id);
    }
    return Postprocessor::DatasetAppearance{};
  };
  tree->appearance_action = [this](const std::vector<Postprocessor::AppearanceTarget>& targets,const Postprocessor::AppearancePatch& patch)
  {
    const auto app = this->application.lock();
    if (!app)
    {
      return;
    }
    std::string error;
    if (!app->change_dataset_appearance(targets,patch,error,true))
    {
      PRINT_ERROR("Cannot change appearance: %s\n",error.c_str());
      return;
    }
    const auto commands = Postprocessor::appearance_commands(*app,targets,patch,error);
    for (const auto& command : commands)
    {
      if (!submitCommand || !submitCommand(command.toStdString(),error))
      {
        break;
      }
    }
    if (!error.empty())
    {
      PRINT_ERROR("Cannot change appearance: %s\n",error.c_str());
    }
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
      viewport->request_redraw();
    });
  };
  tree->appearance_selection = [this](const std::vector<Postprocessor::AppearanceTarget>& targets)
  {
    properties->set_appearance_targets(targets);
  };
  tree->ids_action = [this](bool shown)
  {
    QString state = "off";
    if (shown)
    {
      state = "on";
    }
    this->submit("post tree ids "+state);
  };
  create_result_properties_pages();
  create_toolbars(host);
  refresh();
}

PostprocessorWorkspace::~PostprocessorWorkspace() {
  delete nativeToolbar;
  delete sourceToolbar;
  delete toolbar;
  delete viewsToolbar;
  delete pickingToolbar;
  delete unitsToolbar;
  delete captureToolbar;
  delete resultsDock;
  delete navigationDock;
  delete fieldToolbar;
  delete propertiesDock;
  if (const auto app = application.lock())
  {
    app->set_properties_handler({});
  }
  // The options dialog may have reparented this page into its panel stack.
  delete optionsPanel.data();
  delete mouseOptionsPanel.data();
}

bool PostprocessorWorkspace::camera_command_active() const noexcept
{
  return animationCameraCommand || (viewport != nullptr && viewport->camera_command_active());
}

void PostprocessorWorkspace::prepare_model_reset()
{
  ++explosionEditGeneration;
  // Model notifications must not submit nested Cubit commands or restore previews.
  navigation->reset_navigation();
  deformationControls->discard_pending();
  clipControls->discard_pending();
  sectionControls->discard_pending();
  explosionControls->discard_pending();
  viewport->prepare_model_reset();
  panelRevision.reset();
}

void PostprocessorWorkspace::refresh() {
  const QScopedValueRollback<bool> guard(refreshingWorkspace,true);
  annotationControls->refresh();
  clipControls->refresh();
  sectionControls->refresh();
  thresholdControls->refresh();
  explosionControls->refresh();
  vectorLegendProperties->refresh();
  bool vectorsEnabled = false;
  if (const auto app = application.lock())
  {
    std::string vectorError;
    const auto vectors = app->vector_plot({},vectorError);
    vectorsEnabled = vectors && vectors->enabled;
    const auto view = app->session().views.find(app->session().active_view_id);
    if (view != app->session().views.end())
    {
      const QSignalBlocker components(explodeAction);
      const QSignalBlocker mesh(explodeMeshAction);
      explodeAction->setChecked(view->second.explosion.enabled);
      explodeMeshAction->setChecked(std::any_of(view->second.explosion.meshes.begin(),view->second.explosion.meshes.end(),[](const auto& entry)
      {
        return entry.second.enabled;
      }));
    }
  }
  {
    const QSignalBlocker blocker(vectorAction);
    vectorAction->setChecked(vectorsEnabled);
  }
  try {
    const auto active = application.lock();

    if (!active) {
      return;
    }

    if (active->animation_capture_active() && !animationProgress)
    {
      show_animation_progress();
    }
    const auto& state = active->session();
    const auto currentView = state.views.find(state.active_view_id);
    std::uint64_t selectionRevision = 0;
    if (currentView != state.views.end()) {
      selectionRevision = currentView->second.selection.revision;
    }
    const bool plotChanged = !panelPlotRevision || *panelPlotRevision != state.plot_revision;
    const bool plotActive = currentView != state.views.end() && currentView->second.plot_active;
    const bool dataChanged = panelPlotActive != plotActive || !panelSelectionRevision || *panelSelectionRevision != selectionRevision ||
        !panelRevision || *panelRevision != active->display_revision() || panelSource != state.active_source_id || panelView != state.active_view_id;
    if (dataChanged || plotChanged) {
      refresh_result_panes(active,dataChanged);
      panelPlotRevision = state.plot_revision;
    }
    if (dataChanged) {
      tree->refresh(state, [active](const std::string& id) {
        return active->result_data(id);
      }, [active](const std::string& id, std::size_t index) {
        return active->frame_has_mesh(id, index);
      },[active](const std::string& id,std::optional<std::size_t> frame) {
        return active->field_groups(id,frame);
      },[active](const std::string& id) {
        return active->mesh_datasets(id);
      },[active](const std::string& id) {
        return active->tables(id);
      },[active](const std::string& id) {
        return active->source_unavailable_reason(id);
      });
      const bool table = currentView != state.views.end() && currentView->second.table.has_value() && !currentView->second.plot_active;
      viewTitle->setText(tr("3D View 1"));
      refresh_properties(active);
      if (scaleAction)
      {
        const QSignalBlocker blocker(scaleAction);
        bool visible = false;
        if (currentView != state.views.end())
        {
          visible = currentView->second.scale_visible;
        }
        scaleAction->setChecked(visible);
      }
      colorControls->refresh();
      deformationControls->refresh();
      vectorControls->refresh();
      pickControls->refresh();
      pickControls->setEnabled(true);
      const bool spatialSource = active->source_has_mesh(state.active_source_id);
      colorControls->setEnabled(spatialSource);
      deformationControls->setEnabled(spatialSource);
      vectorControls->setEnabled(spatialSource && vectorControls->isEnabled());
      panelPlotActive = plotActive;
      panelPlotRevision = state.plot_revision;
      panelSelectionRevision = selectionRevision;
      panelRevision = active->display_revision();
      panelSource = state.active_source_id;
      panelView = state.active_view_id;
      std::optional<std::size_t> frame;
      Postprocessor::AnimationDefinition animation;
      for (const auto& entry : state.representations) {
        const auto& display = entry.second;
        if (active->display_matches_source(display,panelSource,panelView)) {
          frame = display.increment_index;
          animation = display.animation;
          break;
        }
      }
      if (table && currentView->second.table->source_id == panelSource) {
        frame = currentView->second.table->frame;
      }
      navigation->refresh(active->result_data(panelSource), frame, [active, id = panelSource](std::size_t index) {
        return active->frame_available(id, index);
      },panelSource+":"+panelView,animation,!table);
    }
    plotPane->synchronize();
    viewport->synchronize(active);
  }
  catch (const std::exception& exception) {
    PRINT_ERROR("Cannot refresh Postprocessor workspace: %s\n", exception.what());
  }
  catch (...) {
    PRINT_ERROR("Cannot refresh Postprocessor workspace: unknown failure.\n");
  }
}

void PostprocessorWorkspace::workspace_view(WorkspaceView& view) {
  view.id = "postprocessor";
  view.label = "Postprocessor";
  const auto activeApplication = application.lock();
  if (activeApplication != nullptr) {
    view.icon = PostprocessorIconLoader::load(activeApplication->active_icons_path(), "Postprocessor");
  } else {
    view.icon = QIcon();
  }
  view.optionsPage = optionsPanel;
  view.refreshOptions = [panel = QPointer<PostprocessorOptionsPanel>(optionsPanel)]() {
    if (panel != nullptr) {
      panel->refresh_settings();
    }
  };
  view.applyOptions = [panel = QPointer<PostprocessorOptionsPanel>(optionsPanel)](std::string& error) {
    if (panel == nullptr) {
      error = "The Postprocessor options page is unavailable.";
      return false;
    }
    return panel->save_settings(error);
  };
  view.additionalOptionsPages =
  {
    {QStringLiteral("Postprocessor Mouse"), mouseOptionsPanel.data(),
      [panel = mouseOptionsPanel]() {
        if (panel != nullptr) {
          panel->refresh_settings();
        }
      },
      [panel = mouseOptionsPanel](std::string& error) {
        if (panel == nullptr) {
          error = "The Postprocessor Mouse options page is unavailable.";
          return false;
        }
        return panel->save_settings(error);
      }}
  };
  view.graphicsPage = this;
  view.useNativeNavigation = false;
  view.dockLayout = WorkspaceDockLayout::LeftRight;
  view.docks = {resultsDock,propertiesDock,navigationDock};
  view.graphicsBottomDock = navigationDock;
  view.graphicsBottomDockRevision = 4;
  view.menuActions = menus;
  view.toolbars = {nativeToolbar,toolbar,pickingToolbar,viewsToolbar,unitsToolbar,captureToolbar,sourceToolbar,fieldToolbar};
  view.modelManagementToolbar = unitsToolbar;
  view.toolbarRowBreaks = {sourceToolbar};
  view.activationChanged = [page = QPointer<PostprocessorWorkspace>(this)](bool active) {
    if (active && page != nullptr) {
      page->sourceControls->refresh();
      page->refresh();
    } else if (page != nullptr) {
      page->viewport->cancel_interaction();
    }
  };
}

void PostprocessorWorkspace::add_job(const SimResults::ResultJobEntry& job) {
  // Prefix is a registered command token, not user-entered display text.
  if (!job.available || job.reference.job_id <= 0 || job.command_prefix.empty() ||
      !std::all_of(job.command_prefix.begin(),job.command_prefix.end(),[](unsigned char value) {
        return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
          (value >= '0' && value <= '9') || value == '_';
      })) {
    return;
  }
  submit(QStringLiteral("post open ")+QString::fromStdString(job.command_prefix)+" job "+QString::number(job.reference.job_id));
  QTimer::singleShot(0,this,[this]() {
    sourceControls->refresh();
    panelRevision.reset();
    refresh();
  });
}
void PostprocessorWorkspace::import_results() {
  const auto command = PostprocessorImportDialog::command(this);
  if (!command.isEmpty()) {
    submit(command);
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  }
}

void PostprocessorWorkspace::refresh_result_panes(const std::shared_ptr<PostprocessorApplicationInterface>& active,bool dataChanged) {
  const auto& state = active->session();
  const auto currentView = state.views.find(state.active_view_id);
  const bool table = currentView != state.views.end() && currentView->second.table.has_value();
  if (table && dataChanged) {
    viewport->cancel_interaction();
    tableView->show_data(active->table_data());
    tableView->set_plot_reference(currentView->second.table);
  } else if (!table) {
    tableView->show_data({});
    tableView->set_plot_reference(std::nullopt);
  }
  const QSignalBlocker blockTabs(resultTabs);
  const bool plot = currentView != state.views.end() && currentView->second.plot_id.has_value();
  const bool paneChanged = resultTabs->isHidden() == (table || plot);
  if (table && resultTabs->indexOf(tableView) < 0) {
    resultTabs->addTab(tableView,tr("Table"));
    resultTabs->setCurrentWidget(tableView);
  }
  if (!table && resultTabs->indexOf(tableView) >= 0) {
    resultTabs->removeTab(resultTabs->indexOf(tableView));
    tableView->hide();
  }
  if (plot && resultTabs->indexOf(plotPane) < 0) {
    resultTabs->addTab(plotPane,tr("Plot"));
    resultTabs->setCurrentWidget(plotPane);
  }
  if (!plot && resultTabs->indexOf(plotPane) >= 0) {
    resultTabs->removeTab(resultTabs->indexOf(plotPane));
    plotPane->hide();
  }
  if (plot && (!table || currentView->second.plot_active)) {
    resultTabs->setCurrentWidget(plotPane);
  } else if (table) {
    resultTabs->setCurrentWidget(tableView);
  }
  resultTabs->setVisible(table || plot);
  viewport->setVisible(true);
  if ((table || plot) && !tablePaneInitialized) {
    const int available = std::max(450,viewSplitter->width());
    viewSplitter->setSizes({available*2/3,available/3});
    tablePaneInitialized = true;
  }
  if (paneChanged) {
    QTimer::singleShot(0,viewport,[page = QPointer<PostprocessorViewport>(viewport)]() {
      if (page) {
        page->request_redraw();
      }
    });
  }
}
