#include <QScrollArea>
#include <QStackedWidget>
#include <cctype>
#include "Postprocessor/GUI/Properties/PostprocessorDisplayProperties.hpp"
#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/GUI/Filters/Clip/PostprocessorClipControls.hpp"
#include "Postprocessor/GUI/Filters/Section/PostprocessorSectionControls.hpp"
#include "Postprocessor/Commands/tables/TableCommandText.hpp"
#include "Postprocessor/GUI/Capture/PostprocessorCaptureDialog.hpp"
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
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationControls.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorProperties.hpp"
#include "Postprocessor/Commands/display/AppearanceCommandText.hpp"
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
#include <QTabWidget>
#include <QSignalBlocker>
#include <QToolButton>
#include <QInputDialog>
#include <QLineEdit>
#include "Postprocessor/GUI/Plots/PostprocessorPlotPane.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotControls.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include <QMenuBar>
#include <QPointer>
#include <QStyle>
#include <QSplitter>
#include <QFileDialog>
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
  });
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
  resultsDock->setWidget(tree);
  resultsDock->hide();

  propertiesDock = new QDockWidget(tr("Properties"), host);
  propertiesDock->setObjectName("Postprocessor.PropertiesDock");
  propertiesPages = new QStackedWidget(propertiesDock);
  emptyProperties = new QWidget(propertiesPages);
  propertiesPages->addWidget(emptyProperties);
  properties = new PostprocessorProperties(this->application,propertiesPages,[this](const std::string& command,std::string& error)
  {
    if (!submitCommand || !submitCommand(command,error))
    {
      return false;
    }
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
      viewport->request_redraw();
    });
    return true;
  });
  propertiesPages->addWidget(properties);
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
  legendProperties = new PostprocessorDisplayProperties(this->application,submitCommand,propertiesPages);
  auto* legendScroll = new QScrollArea(propertiesPages);
  legendScroll->setWidgetResizable(true);
  legendScroll->setFrameShape(QFrame::NoFrame);
  legendScroll->setWidget(legendProperties);
  legendEditorPage = legendScroll;
  propertiesPages->addWidget(legendEditorPage);
  propertiesDock->setWidget(propertiesPages);
  propertiesDock->hide();

  nativeToolbar = new PostprocessorToolbar(host);
  nativeToolbar->hide();
  sourceToolbar = new QToolBar(tr("Result sources"),host);
  sourceToolbar->setObjectName("Postprocessor.SourceToolbar");
  sourceToolbar->setAllowedAreas(Qt::TopToolBarArea);
  sourceToolbar->setMovable(true);
  sourceToolbar->setFloatable(true);
  const auto catalogue = [this](std::vector<SimResults::ResultJobEntry>& entries,std::string& error) {
    const auto active = this->application.lock();
    return active && active->available_jobs(entries,error);
  };
  sourceControls = new PostprocessorSourceControls(catalogue,[this](const SimResults::ResultJobEntry& job) {
    add_job(job);
  },[this]() {
    import_results();
  },sourceToolbar);
  sourceToolbar->addWidget(sourceControls);
  sourceToolbar->hide();
  tree->available_jobs = catalogue;
  tree->add_job = [this](const SimResults::ResultJobEntry& job) {
    add_job(job);
  };
  tree->import_files = [this]() {
    import_results();
  };
  tree->source_action = [this](const QString& source,SourceAction action) {
    if (action == SourceAction::RemoveAll) {
      if (QMessageBox::question(this,tr("Remove sources"),tr("Remove all opened Postprocessor sources? Jobs and files are retained.")) == QMessageBox::Yes) {
        this->submit("post close all");
      }
      return;
    }
    QString verb = "close";
    if (action == SourceAction::Show) {
      verb = "show";
    } else if (action == SourceAction::Hide) {
      verb = "hide";
    } else if (action == SourceAction::ShowOnly) {
      verb = "show only";
    } else if (action == SourceAction::Refresh) {
      verb = "refresh";
    }
    source_command(verb,source);
    QTimer::singleShot(0,this,[this]() {
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

  viewsToolbar = new QToolBar(tr("Views"),host);
  viewsToolbar->setObjectName("Postprocessor.ViewsToolbar");
  viewsToolbar->setAllowedAreas(Qt::TopToolBarArea);
  viewsToolbar->setMovable(true);
  viewsToolbar->setFloatable(true);
  viewsToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  viewsToolbar->hide();

  pickingToolbar = new QToolBar(tr("Picking"),host);
  pickingToolbar->setObjectName("Postprocessor.PickingToolbar");
  pickingToolbar->setAllowedAreas(Qt::TopToolBarArea);
  pickingToolbar->setMovable(true);
  pickingToolbar->setFloatable(true);
  pickingToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  pickingToolbar->hide();

  unitsToolbar = new QToolBar(tr("Units"),host);
  unitsToolbar->setObjectName("Postprocessor.UnitsToolbar");
  unitsToolbar->setAllowedAreas(Qt::TopToolBarArea);
  unitsToolbar->setMovable(true);
  unitsToolbar->setFloatable(true);
  unitsToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  unitsToolbar->hide();

  captureToolbar = new QToolBar(tr("Capture"),host);
  captureToolbar->setObjectName("Postprocessor.CaptureToolbar");
  captureToolbar->setAllowedAreas(Qt::TopToolBarArea);
  captureToolbar->setMovable(true);
  captureToolbar->setFloatable(true);
  captureToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  captureToolbar->hide();

  QIcon fitIcon(":/cubitgui/zoomreset.svg");
  if (fitIcon.isNull()) {
    fitIcon = style()->standardIcon(QStyle::SP_TitleBarMaxButton);
  }
  auto* fit = new QAction(fitIcon, tr("Fit visible results"), this);
  fit->setToolTip(tr("Fit all visible meshes in the active Postprocessor view"));
  connect(fit, &QAction::triggered, this, [this]() {
    viewport->fit_view();
  });
  viewsToolbar->addAction(fit);
  const char* captureIcons[] = {"PostprocessorCaptureViewport","PostprocessorRenderImage","PostprocessorAddToReport"};
  const QString captureTips[] = {tr("Save viewport screenshot"),tr("Render PNG image"),tr("Add image to Report")};
  for (int index = 0; index < 3; ++index)
  {
    const auto app = this->application.lock();
    QIcon icon;
    if (app != nullptr)
    {
      icon = PostprocessorIconLoader::load(app->active_icons_path(), captureIcons[index]);
    }
    auto* action = captureToolbar->addAction(icon, captureTips[index]);
    action->setToolTip(captureTips[index]);
    connect(action,&QAction::triggered,this,[this,index]()
    {
      capture_dialog(index == 2,index == 0);
    });
  }
  pickControls = new PostprocessorPickControls(this->application,[this](const QString& command) {
    this->submit(command);
    QTimer::singleShot(0,this,[this]() {
      panelSelectionRevision.reset();
      refresh();
    });
  },pickingToolbar);
  pickingToolbar->addWidget(pickControls);
  QIcon legendIcon;
  if (const auto app = this->application.lock())
  {
    legendIcon = PostprocessorIconLoader::load(app->active_icons_path(),"PostprocessorLegend");
  }
  auto* openLegend = toolbar->addAction(legendIcon,tr("Legend"));
  openLegend->setToolTip(tr("Open Legend properties in the right dock"));
  connect(openLegend,&QAction::triggered,this,[this]()
  {
    this->submit("post properties legend");
  });
  annotationControls = new PostprocessorAnnotationControls(this->application,[this](const QString& command) {
    this->submit(command);
    viewport->request_redraw();
    annotationControls->refresh();
  },toolbar);
  toolbar->addWidget(annotationControls);
  auto* plotHistory = new QToolButton(toolbar);
  plotHistory->setObjectName("Postprocessor.PlotHistory");
  plotHistory->setAccessibleName(tr("Plot result history"));
  const auto plotApplication = this->application.lock();
  if (plotApplication) {
    plotHistory->setIcon(PostprocessorIconLoader::load(plotApplication->active_icons_path(),"PostprocessorPlotHistory"));
  }
  plotHistory->setIconSize(toolbar->iconSize());
  connect(toolbar,&QToolBar::iconSizeChanged,plotHistory,&QToolButton::setIconSize);
  plotHistory->setAutoRaise(true);
  plotHistory->setPopupMode(QToolButton::MenuButtonPopup);
  plotHistory->setToolTip(tr("Plot selected entity or open a saved plot"));
  auto* plotsMenu = new QMenu(plotHistory);
  plotHistory->setMenu(plotsMenu);
  connect(plotsMenu,&QMenu::aboutToShow,this,[this,plotsMenu]() {
    plotsMenu->clear();
    const auto active = this->application.lock();
    if (!active) {
      return;
    }
    auto* create = plotsMenu->addAction(tr("New plot..."));
    connect(create,&QAction::triggered,this,[this]() {
      bool ok = false;
      const auto name = QInputDialog::getText(this,tr("New plot"),tr("Name"),QLineEdit::Normal,{},&ok);
      if (ok && !Postprocessor::plot_quote(name.toUtf8().toStdString()).isEmpty()) {
        this->submit(QStringLiteral("post plot create ")+Postprocessor::plot_quote(name.toUtf8().toStdString()));
        refresh();
      }
    });
    for (const auto& entry : active->session().plots) {
      const auto name = entry.second.name;
      auto* action = plotsMenu->addAction(QString::fromUtf8(name.c_str()));
      connect(action,&QAction::triggered,this,[this,name]() {
        this->submit(QStringLiteral("post plot show ")+Postprocessor::plot_quote(name));
        refresh();
      });
    }
  });
  connect(plotHistory,&QToolButton::clicked,this,[this,plotHistory,plotsMenu]() {
    const auto active = this->application.lock();
    if (!active) {
      return;
    }
    const auto view = active->session().views.find(active->session().active_view_id);
    if (view == active->session().views.end() || !view->second.selection.entity) {
      plotsMenu->exec(plotHistory->mapToGlobal(QPoint(0,plotHistory->height())));
      return;
    }
    const auto entity = *view->second.selection.entity;
    const auto commands = Postprocessor::plot_entity_commands(*active,entity);
    if (commands.empty()) {
      QMenu menu(this);
      add_plot_context_actions(menu,*active,entity,[this](const QString& command) {
        this->submit(command);
        refresh();
      });
      menu.exec(plotHistory->mapToGlobal(QPoint(0,plotHistory->height())));
      return;
    }
    for (const auto& command : commands) {
      this->submit(command);
    }
    refresh();
  });
  toolbar->addWidget(plotHistory);
  fieldToolbar = new QToolBar(tr("Field selection"),host);
  fieldToolbar->setObjectName("Postprocessor.FieldToolbar");
  fieldToolbar->setAllowedAreas(Qt::TopToolBarArea);
  fieldToolbar->setMovable(true);
  fieldToolbar->setFloatable(true);
  colorControls = new PostprocessorColorControls(this->application,[this](const QString& command) {
    this->submit(command);
    QTimer::singleShot(0,this,[this]() {
      panelRevision.reset();
      refresh();
    });
  },fieldToolbar);
  fieldToolbar->addWidget(colorControls);
  fieldToolbar->hide();

  auto* view = new QMenu(tr("View"), this);
  view->addAction(viewsToolbar->toggleViewAction());
  view->addAction(pickingToolbar->toggleViewAction());
  view->addAction(unitsToolbar->toggleViewAction());
  view->addAction(captureToolbar->toggleViewAction());
  view->addAction(sourceToolbar->toggleViewAction());
  view->addAction(fieldToolbar->toggleViewAction());
  view->addAction(deformationToolbar->toggleViewAction());
  view->addAction(navigationDock->toggleViewAction());

  auto dispatchFilter = [this](const QString& command)
  {
    this->submit(command);
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
      viewport->request_redraw();
    });
  };
  auto previewFilter = [this](const QString& command)
  {
    std::string error;
    if (silentCommand && !silentCommand(command.toUtf8().toStdString(),error))
    {
      PRINT_ERROR("Cannot preview plane: %s\n",error.c_str());
    }
    QTimer::singleShot(0,this,[this]()
    {
      panelRevision.reset();
      refresh();
      viewport->request_redraw();
    });
  };
  clipControls = new PostprocessorClipControls(this->application,dispatchFilter,propertiesPages,previewFilter);
  sectionControls = new PostprocessorSectionControls(this->application,dispatchFilter,propertiesPages,previewFilter);
  auto* clipScroll = new QScrollArea(propertiesPages);
  clipScroll->setWidgetResizable(true);
  clipScroll->setFrameShape(QFrame::NoFrame);
  clipScroll->setWidget(clipControls);
  clipEditorPage = clipScroll;
  propertiesPages->addWidget(clipEditorPage);
  auto* sectionScroll = new QScrollArea(propertiesPages);
  sectionScroll->setWidgetResizable(true);
  sectionScroll->setFrameShape(QFrame::NoFrame);
  sectionScroll->setWidget(sectionControls);
  sectionEditorPage = sectionScroll;
  propertiesPages->addWidget(sectionEditorPage);
  if (const auto app = this->application.lock())
  {
    app->set_properties_handler([this](const std::string& page,const std::string& source,std::string& error)
    {
      return show_properties(page,source,error);
    });
  }
  for (const auto* tool : {"Clip","Section"})
  {
    QIcon icon;
    if (const auto app = this->application.lock())
    {
      icon = PostprocessorIconLoader::load(app->active_icons_path(),std::string("Postprocessor")+tool);
    }
    auto* action = toolbar->addAction(icon,QString::fromLatin1(tool));
    action->setToolTip(tr("Open %1 properties in the right dock").arg(QString::fromLatin1(tool)));
    std::string page(tool);
    std::transform(page.begin(),page.end(),page.begin(),[](unsigned char c)
    {
      return static_cast<char>(std::tolower(c));
    });
    connect(action,&QAction::triggered,this,[this,page]()
    {
      this->submit(QString::fromStdString("post properties "+page));
    });
  }
  const char* viewNames[] = {"Front","Back","Top","Bottom","Left","Right","Iso","Ortho","Perspective"};
  const char* viewCommands[] = {"front","back","top","bottom","left","right","iso","projection parallel","projection perspective"};
  for (int i = 0; i < 9; ++i)
  {
    QIcon icon;
    if (const auto app = this->application.lock())
    {
      icon = PostprocessorIconLoader::load(app->active_icons_path(),std::string("PostprocessorView")+viewNames[i]);
    }
    auto* action = viewsToolbar->addAction(icon,QString::fromLatin1(viewNames[i]));
    action->setToolTip(tr("Postprocessor view: %1").arg(QString::fromLatin1(viewNames[i])));
    const QString command = QStringLiteral("post view ")+QString::fromLatin1(viewCommands[i]);
    connect(action,&QAction::triggered,this,[this,command]()
    {
      camera_tool(command);
    });
  }
  QIcon scaleIcon;
  if (const auto app = this->application.lock())
  {
    scaleIcon = PostprocessorIconLoader::load(app->active_icons_path(),"PostprocessorScale");
  }
  scaleAction = viewsToolbar->addAction(scaleIcon,tr("Unit-aware scale"));
  scaleAction->setCheckable(true);
  scaleAction->setToolTip(tr("Show the viewport distance scale in presentation length units"));
  connect(scaleAction,&QAction::triggered,this,[this](bool checked)
  {
    if (checked)
    {
      this->submit("post scale on");
    }
    else
    {
      this->submit("post scale off");
    }
  });
  // Framework supplies the shared Help menu and keeps it last.
  host->menuBar()->addAction(view->menuAction());
  view->menuAction()->setVisible(false);
  menus.append(view->menuAction());
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

void PostprocessorWorkspace::submit(const QString& command) {
  viewport->cancel_interaction();
  plotPane->cancel();
  std::string error;

  if (!submitCommand || !submitCommand(command.toUtf8().toStdString(), error)) {
    PRINT_ERROR("Cannot submit Postprocessor command: %s\n", error.c_str());
  }
}

QString PostprocessorWorkspace::frame_command(std::size_t frame) const
{
  const auto active = application.lock();
  if (!active) {
    return {};
  }
  const auto& state = active->session();
  const auto source = state.sources.find(state.active_source_id);
  std::optional<std::size_t> current;
  for (const auto& entry : state.representations) {
    const auto& display = entry.second;
    if (active->display_matches_source(display,state.active_source_id,state.active_view_id)) {
      current = display.increment_index;
      break;
    }
  }
  const auto view = state.views.find(state.active_view_id);
  if (view != state.views.end() && view->second.table && view->second.table->source_id == state.active_source_id) {
    current = view->second.table->frame;
  }
  if (source == state.sources.end() || !navigation->matches_context(state.active_source_id+":"+state.active_view_id,
      active->result_data(state.active_source_id),current)) {
    return {};
  }
  const auto name = QString::fromUtf8(source->second.name.c_str());
  QChar delimiter('"');
  if (name.contains(delimiter)) {
    delimiter = QChar(0x27);
  }
  if (name.isEmpty() || name.contains(delimiter)) {
    return {};
  }
  for (QChar c : name) {
    if (c.unicode() < 32 || c.unicode() == 127) {
      return {};
    }
  }
  return "post frame " + QString::number(static_cast<qulonglong>(frame)) + " source " + delimiter + name + delimiter;
}

void PostprocessorWorkspace::source_command(const QString& verb, const QString& name) {
  // Match Cubit's quoted-string syntax without allowing a name to inject a
  // second command. Do not reach into solver-specific GUI quoting utilities.
  QChar delimiter('"');
  if (name.contains(delimiter)) {
    delimiter = QChar(0x27);
  }

  if (name.isEmpty() || name.contains(delimiter)) {
    PRINT_ERROR("The source name cannot be represented as one quoted command argument.\n");
    return;
  }

  for (const auto character : name) {
    if (character.unicode() < 0x20 || character.unicode() == 0x7f) {
      PRINT_ERROR("A source command requires a name without control characters.\n");
      return;
    }
  }

  submit("post " + verb + " " + delimiter + name + delimiter);
}

void PostprocessorWorkspace::dataset_command(const QString& name, Postprocessor::DatasetVisibilityAction action,
  std::optional<std::size_t> dataset) {
  const auto quote = [](const QString& text) -> QString
  {
    QChar delimiter('"');
    if (text.contains(delimiter)) {
      delimiter = QChar(0x27);
    }
    if (text.isEmpty() || text.contains(delimiter)) {
      return {};
    }
    for (const auto character : text) {
      if (character.unicode() < 32 || character.unicode() == 127) {
        return {};
      }
    }
    return delimiter+text+delimiter;
  };
  const auto source = quote(name);
  const auto active = application.lock();
  if (!active || source.isEmpty()) {
    PRINT_ERROR("Dataset visibility requires a valid source name.\n");
    return;
  }
  std::string sourceId;
  for (const auto& entry : active->session().sources) {
    if (QString::fromUtf8(entry.second.name.c_str()) == name) {
      sourceId = entry.first;
      break;
    }
  }
  using Action = Postprocessor::DatasetVisibilityAction;
  QString selector = "all";
  if (action != Action::ShowAll) {
    if (!dataset) {
      return;
    }
    const auto catalogue = active->mesh_datasets(sourceId);
    const auto found = std::find_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value) {
      return value.id == *dataset;
    });
    if (found == catalogue.end()) {
      PRINT_ERROR("The selected mesh dataset is no longer available.\n");
      return;
    }
    selector = QString::number(static_cast<qulonglong>(*dataset));
    const auto label = quote(QString::fromUtf8(found->name.c_str()));
    if (!label.isEmpty() && std::count_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value) {
          return value.name == found->name;
        }) == 1) {
      selector = label;
    }
  }
  QString operation = "show dataset ";
  if (action == Action::Hide) {
    operation = "hide dataset ";
  } else if (action == Action::ShowOnly) {
    operation = "show only dataset ";
  }
  submit("post "+operation+selector+" source "+source);
}

bool PostprocessorWorkspace::camera_command_active() const noexcept
{
  return viewport != nullptr && viewport->camera_command_active();
}

void PostprocessorWorkspace::refresh() {
  annotationControls->refresh();
  clipControls->refresh();
  sectionControls->refresh();
  try {
    const auto active = application.lock();

    if (!active) {
      return;
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
      pickControls->refresh();
      pickControls->setEnabled(true);
      const bool spatialSource = active->source_has_mesh(state.active_source_id);
      colorControls->setEnabled(spatialSource);
      deformationControls->setEnabled(spatialSource);
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

bool PostprocessorWorkspace::bind_native_toolbar(Claro* host, std::string& error) {
  if (!nativeToolbar->bind_native_actions(host,error)) {
    return false;
  }
  sourceControls->set_import_icon(nativeToolbar->open_icon());
  return true;
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

std::array<int,2> PostprocessorWorkspace::capture_reference_size(Postprocessor::CaptureTarget target,const std::string& id) const
{
  const auto active = application.lock();
  if (active && target == Postprocessor::CaptureTarget::View && id == active->session().active_view_id) {
    return {viewport->width(),viewport->height()};
  }
  if (active && target == Postprocessor::CaptureTarget::Plot) {
    const auto view = active->session().views.find(active->session().active_view_id);
    if (view != active->session().views.end() && view->second.plot_id == id) {
      const auto size = plotPane->chart_size();
      return {size.width(),size.height()};
    }
  }
  return {Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
}
void PostprocessorWorkspace::capture_dialog(bool report,bool quick) {
  const auto active = application.lock();
  if (!active) {
    return;
  }
  QString command;
  if (quick) {
    auto file = QFileDialog::getSaveFileName(this,tr("Save viewport screenshot"),{},tr("PNG images (*.png)"),nullptr,QFileDialog::DontConfirmOverwrite);
    if (file.isEmpty()) {
      return;
    }
    if (!file.endsWith(".png",Qt::CaseInsensitive)) {
      file += ".png";
    }
    const int width = qRound(viewport->width()*viewport->devicePixelRatioF());
    const int height = qRound(viewport->height()*viewport->devicePixelRatioF());
    command = "post capture view " + Postprocessor::plot_quote(active->session().active_view_id) +
      " file " + Postprocessor::plot_quote(file.toUtf8().toStdString()) + QString(" width %1 height %2").arg(width).arg(height);
  } else {
    command = postprocessor_capture_dialog(this,*active,report);
  }
  if (!command.isEmpty()) {
    submit(command);
  }
}

bool PostprocessorWorkspace::show_properties(const std::string& page,const std::string& source,std::string& error)
{
  error.clear();
  const auto app = this->application.lock();
  if (!app)
  {
    error = "The Postprocessor workspace is unavailable.";
    return false;
  }
  if (page == "treeids-on" || page == "treeids-off")
  {
    tree->setColumnHidden(1,page == "treeids-off");
    return true;
  }
  QWidget* selected = emptyProperties;
  QString title = tr("Properties");
  if (page == "source")
  {
    const auto found = app->session().sources.find(source);
    if (found == app->session().sources.end())
    {
      error = "The inspected source is unavailable.";
      return false;
    }
    inspectedSource = source;
    properties->show_source(&found->second);
    selected = properties;
    title = tr("Source properties");
  }
  else if (page == "legend")
  {
    selected = legendEditorPage;
    legendProperties->refresh();
    title = tr("Legend properties");
  }
  else if (page == "clip")
  {
    selected = clipEditorPage;
    clipControls->refresh();
    title = tr("Clip properties");
  }
  else if (page == "section")
  {
    selected = sectionEditorPage;
    sectionControls->refresh();
    title = tr("Section properties");
  }
  propertiesPage = page;
  if (page != "source")
  {
    inspectedSource.clear();
  }
  propertiesPages->setCurrentWidget(selected);
  propertiesDock->setWindowTitle(title);
  if (page != "close")
  {
    propertiesDock->show();
    propertiesDock->raise();
  }
  return true;
}
void PostprocessorWorkspace::refresh_properties(const std::shared_ptr<PostprocessorApplicationInterface>& app)
{
  properties->set_appearance_targets(tree->appearance_targets());
  if (propertiesPage == "source")
  {
    const auto source = app->session().sources.find(inspectedSource);
    if (source == app->session().sources.end())
    {
      propertiesPage.clear();
      inspectedSource.clear();
      propertiesPages->setCurrentWidget(emptyProperties);
    }
    else
    {
      properties->show_source(&source->second);
    }
  }
  else if (propertiesPage == "legend")
  {
    legendProperties->refresh();
  }
}
void PostprocessorWorkspace::camera_tool(const QString& command)
{
  const auto app = this->application.lock();
  if (!app)
  {
    return;
  }
  if (app->user_options().journalCamera)
  {
    this->submit(command);
  }
  else if (silentCommand)
  {
    std::string error;
    if (!silentCommand(command.toUtf8().toStdString(),error))
    {
      PRINT_ERROR("Cannot change standard view: %s\n",error.c_str());
    }
    viewport->request_redraw();
  }
}
