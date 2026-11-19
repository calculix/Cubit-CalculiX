#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/GUI/Explosion/PostprocessorExplosionControls.hpp"
#include "Postprocessor/GUI/Explosion/PostprocessorExplosionPlayback.hpp"
#include "Postprocessor/GUI/Filters/Clip/PostprocessorClipControls.hpp"
#include "Postprocessor/GUI/Filters/Section/PostprocessorSectionControls.hpp"
#include "Postprocessor/GUI/Filters/Threshold/PostprocessorThresholdControls.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorDisplayProperties.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorProperties.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/VectorPlot/PostprocessorVectorControls.hpp"
#include "Postprocessor/GUI/VectorPlot/PostprocessorVectorLegendProperties.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QDockWidget>
#include <QMainWindow>
#include <QScopedValueRollback>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTimer>

void PostprocessorWorkspace::create_properties_dock(QMainWindow* host)
{
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
}

void PostprocessorWorkspace::create_result_properties_pages()
{
  legendProperties = new PostprocessorDisplayProperties(this->application,submitCommand,propertiesPages);
  auto* legendScroll = new QScrollArea(propertiesPages);
  legendScroll->setWidgetResizable(true);
  legendScroll->setFrameShape(QFrame::NoFrame);
  legendScroll->setWidget(legendProperties);
  legendEditorPage = legendScroll;
  propertiesPages->addWidget(legendEditorPage);
  auto* vectorScroll = new QScrollArea(propertiesPages);
  vectorScroll->setWidgetResizable(true);
  vectorScroll->setFrameShape(QFrame::NoFrame);
  vectorScroll->setWidget(vectorControls);
  vectorEditorPage = vectorScroll;
  propertiesPages->addWidget(vectorEditorPage);
  vectorLegendProperties = new PostprocessorVectorLegendProperties(this->application,submitCommand,propertiesPages);
  auto* vectorLegendScroll = new QScrollArea(propertiesPages);
  vectorLegendScroll->setWidgetResizable(true);
  vectorLegendScroll->setFrameShape(QFrame::NoFrame);
  vectorLegendScroll->setWidget(vectorLegendProperties);
  vectorLegendEditorPage = vectorLegendScroll;
  propertiesPages->addWidget(vectorLegendEditorPage);
  propertiesDock->setWidget(propertiesPages);
  propertiesDock->hide();
}

void PostprocessorWorkspace::create_filter_properties_pages()
{
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
  thresholdControls = new PostprocessorThresholdControls(this->application,dispatchFilter,propertiesPages);
  auto* thresholdScroll = new QScrollArea(propertiesPages);
  thresholdScroll->setWidgetResizable(true);
  thresholdScroll->setFrameShape(QFrame::NoFrame);
  thresholdScroll->setWidget(thresholdControls);
  thresholdEditorPage = thresholdScroll;
  propertiesPages->addWidget(thresholdEditorPage);
  const auto queueExplosionEdit = [this](const QString& command,bool preview)
  {
    ++pendingExplosionEdits;
    if (explosionPlayback)
    {
      explosionPlayback->pause_clock();
    }
    const auto generation = explosionEditGeneration;
    QTimer::singleShot(0,this,[this,command,preview,generation]()
    {
      if (generation != explosionEditGeneration)
      {
        --pendingExplosionEdits;
        return;
      }
      if (preview)
      {
        std::string error;
        if (silentCommand && !silentCommand(command.toUtf8().toStdString(),error))
        {
          PRINT_ERROR("Cannot preview explosion: %s\n",error.c_str());
        }
      }
      else
      {
        this->submit(command);
      }
      refresh();
      viewport->request_redraw();
      --pendingExplosionEdits;
    });
  };
  explosionControls = new PostprocessorExplosionControls(this->application,
    [queueExplosionEdit](const QString& command)
    {
      queueExplosionEdit(command,false);
    },
    [queueExplosionEdit](const QString& command)
    {
      queueExplosionEdit(command,true);
    },propertiesPages);
  explosionPlayback = new PostprocessorExplosionPlayback(this->application,[this](const std::string& command,std::string& error)
  {
    // Suppress command-completion panel/tree refreshes for runtime-only ticks,
    // using the same guard as continuous camera rotation.
    const QScopedValueRollback<bool> guard(animationCameraCommand,true);
    return silentCommand && silentCommand(command,error);
  },[this]()
  {
    return viewport->explosion_pulse_ready();
  },[this]()
  {
    return foregroundCommand || refreshingWorkspace || pendingExplosionEdits != 0 || animationCameraCommand;
  },[this]()
  {
    viewport->request_explosion_pulse();
  },this);
  viewport->set_explosion_pulse_step([this]()
  {
    explosionPlayback->advance_for_render();
  });
  auto* explosionScroll = new QScrollArea(propertiesPages);
  explosionScroll->setWidgetResizable(true);
  explosionScroll->setFrameShape(QFrame::NoFrame);
  explosionScroll->setWidget(explosionControls);
  explosionEditorPage = explosionScroll;
  propertiesPages->addWidget(explosionEditorPage);
  if (const auto app = this->application.lock())
  {
    app->set_properties_handler([this](const std::string& page,const std::string& source,std::string& error)
    {
      return show_properties(page,source,error);
    });
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
  else if (page == "vectors")
  {
    selected = vectorEditorPage;
    vectorControls->refresh();
    title = tr("Vector settings");
  }
  else if (page == "vectorlegend")
  {
    selected = vectorLegendEditorPage;
    vectorLegendProperties->refresh();
    title = tr("Vector legend properties");
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
  else if (page == "threshold")
  {
    selected = thresholdEditorPage;
    thresholdControls->refresh();
    title = tr("Threshold properties");
  }
  else if (page == "explosion")
  {
    selected = explosionEditorPage;
    explosionControls->refresh();
    title = tr("Explosion properties");
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
