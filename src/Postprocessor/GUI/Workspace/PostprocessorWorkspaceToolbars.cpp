#include "PostprocessorWorkspace.hpp"
#include "PostprocessorToolbar.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationControls.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotControls.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorColorControls.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPickControls.hpp"
#include "Postprocessor/GUI/Sources/PostprocessorSourceControls.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QAction>
#include <QDockWidget>
#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <algorithm>
#include <cctype>

void PostprocessorWorkspace::create_toolbars(QMainWindow* host)
{
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
  create_capture_toolbar(host);
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
  QIcon vectorIcon;
  if (const auto app = this->application.lock())
  {
    vectorIcon = PostprocessorIconLoader::load(app->active_icons_path(),"PostprocessorVectors");
  }
  vectorAction = fieldToolbar->addAction(vectorIcon,tr("Vectors"));
  vectorAction->setCheckable(true);
  vectorAction->setToolTip(tr("Toggle vectors and open vector settings"));
  connect(vectorAction,&QAction::triggered,this,[this](bool on)
  {
    std::string error;
    if (const auto app = this->application.lock())
    {
      const auto settings = app->vector_plot({},error);
      if (settings)
      {
        if (on)
        {
          this->submit("post vector on");
        }
        else
        {
          this->submit("post vector off");
        }
      }
    }
    if (!show_properties("vectors",{},error))
    {
      PRINT_ERROR("Cannot open vector settings: %s\n",error.c_str());
    }
    refresh();
  });
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

  create_filter_properties_pages();
  for (const auto* tool : {"Clip","Section","Threshold"})
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
  for (bool mesh : {false,true})
  {
    QString title = tr("Explode components");
    std::string file = "PostprocessorExplode";
    if (mesh)
    {
      title = tr("Explode mesh");
      file = "PostprocessorExplodeMesh";
    }
    QIcon icon;
    if (const auto app = this->application.lock())
    {
      icon = PostprocessorIconLoader::load(app->active_icons_path(),file);
    }
    auto* action = toolbar->addAction(icon,title);
    action->setCheckable(true);
    action->setToolTip(tr("Toggle explosion and open its properties"));
    if (mesh)
    {
      explodeMeshAction = action;
    }
    else
    {
      explodeAction = action;
    }
    connect(action,&QAction::triggered,this,[this,mesh](bool on)
    {
      QString command = "post explode components ";
      if (mesh)
      {
        command = "post explode mesh ";
      }
      QString state = "off";
      if (on)
      {
        state = "on";
      }
      this->submit(command+state);
      std::string error;
      if (!show_properties("explosion",{},error))
      {
        PRINT_ERROR("Cannot open explosion properties: %s\n",error.c_str());
      }
      panelRevision.reset();
      refresh();
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
}

bool PostprocessorWorkspace::bind_native_toolbar(Claro* host, std::string& error) {
  if (!nativeToolbar->bind_native_actions(host,error)) {
    return false;
  }
  sourceControls->set_import_icon(nativeToolbar->open_icon());
  return true;
}
