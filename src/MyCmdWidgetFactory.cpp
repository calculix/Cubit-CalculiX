/*!
 *  \file MyCmdWidgetFactory.cpp
 *  \brief
 *    Provides examples of how to use the widget factory to create
 *    command panels.
 *  \author Michael Plooster
 *  \date 24 Feb 2014
 */

#include "MyCmdWidgetFactory.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "NavigationWidgetCache.hpp"
#include "Claro.hpp"

// A list of headers for my command panels.
#include "CCXBlocksElementTypePanel.hpp"
#include "SectionsCreateSolidPanel.hpp"
#include "SectionsCreateShellPanel.hpp"
#include "SectionsCreateBeamPanel.hpp"
#include "SectionsCreateMembranePanel.hpp"
#include "SectionsModifySolidPanel.hpp"
#include "SectionsModifyShellPanel.hpp"
#include "SectionsModifyBeamPanel.hpp"
#include "SectionsModifyMembranePanel.hpp"
#include "SectionsDeletePanel.hpp"
#include "ConstraintsCreateRigidBodyPanel.hpp"
#include "ConstraintsCreateTiePanel.hpp"
#include "ConstraintsModifyRigidBodyPanel.hpp"
#include "ConstraintsModifyTiePanel.hpp"
#include "ConstraintsDeletePanel.hpp"
#include "SurfaceInteractionsCreatePanel.hpp"
#include "SurfaceInteractionsModifyPanel.hpp"
#include "SurfaceInteractionsDeletePanel.hpp"
#include "ContactPairsCreatePanel.hpp"
#include "ContactPairsModifyPanel.hpp"
#include "ContactPairsDeletePanel.hpp"
#include "AmplitudesCreatePanel.hpp"
#include "AmplitudesModifyPanel.hpp"
#include "AmplitudesDeletePanel.hpp"
#include "LoadsForcesModifyPanel.hpp"
#include "LoadsPressuresModifyPanel.hpp"
#include "LoadsHeatfluxesModifyPanel.hpp"
#include "BCsDisplacementsModifyPanel.hpp"
#include "BCsTemperaturesModifyPanel.hpp"
#include "HistoryOutputsCreatePanel.hpp"
#include "HistoryOutputsModifyNodePanel.hpp"
#include "HistoryOutputsModifyElementPanel.hpp"
#include "HistoryOutputsModifyContactPanel.hpp"
#include "HistoryOutputsDeletePanel.hpp"
#include "FieldOutputsCreatePanel.hpp"
#include "FieldOutputsModifyNodePanel.hpp"
#include "FieldOutputsModifyElementPanel.hpp"
#include "FieldOutputsModifyContactPanel.hpp"
#include "FieldOutputsDeletePanel.hpp"
#include "InitialConditionsCreatePanel.hpp"
#include "InitialConditionsModifyPanel.hpp"
#include "InitialConditionsDeletePanel.hpp"
#include "StepsCreatePanel.hpp"
#include "StepsModifyParameterPanel.hpp"
#include "StepsModifyStaticPanel.hpp"
#include "StepsModifyFrequencyPanel.hpp"
#include "StepsModifyBucklePanel.hpp"
#include "StepsModifyHeatTransferPanel.hpp"
#include "StepsModifyCoupledTemperatureDisplacementPanel.hpp"
#include "StepsModifyUncoupledTemperatureDisplacementPanel.hpp"
#include "StepsDeletePanel.hpp"
#include "CustomLinesCreatePanel.hpp"
#include "CustomLinesModifyPanel.hpp"
#include "CustomLinesDeletePanel.hpp"
#include "JobsCreatePanel.hpp"
#include "JobsModifyPanel.hpp"
#include "JobsDeletePanel.hpp"
#include "JobsActionsPanel.hpp"

#include <QWidget>

MyCmdWidgetFactory::MyCmdWidgetFactory(QObject* parent) :
  QObject(parent),
  widget_cache()
{ }

MyCmdWidgetFactory::~MyCmdWidgetFactory()
{
  clear();
}

QWidget* MyCmdWidgetFactory::getWidget(NavigationModel *model,
                                    const QModelIndex &index,
                                    QWidget *widgetParent)
{
  if(!model || !index.isValid() || !widgetParent)
      return 0;

  // Get the marker name for the index.
  NavigationNode *node = model->getNode(index);
  QString name = node->getMarker();

  // If there isn't a marker, use the path.
  if(name.isEmpty())
    name = model->getPath(node);

  // See if the widget already exists.
  QWidget *widget = 0;
  QMap<QWidget *, NavigationWidgetCache *>::Iterator iter =
      widget_cache.find(widgetParent);
  if(iter != widget_cache.end())
  {
    widget = (*iter)->getWidget(name);
    if(widget)
      return widget;
  }

  // Create a new widget based on the name.
  widget = createWidget(name);
  if(widget)
  {
      // Add a new cache for the parent widget if needed.
      NavigationWidgetCache *cache = 0;
      if(iter == widget_cache.end())
      {
        cache = new NavigationWidgetCache(this);
        widget_cache.insert(widgetParent, cache);
        connect(widgetParent, SIGNAL(destroyed(QObject*)),
                this, SLOT(removeCache(QObject*)));
      }
      else
        cache = *iter;

      // Add the widget to the cache.
      cache->insert(name, widget);
  }

  return widget;
}

// We want to be sure to clean up all the memory we allocated.
void MyCmdWidgetFactory::clear()
{
  // Clean up our widget cache if the create_widget_using_cache example was used.
  QMap<QWidget*, NavigationWidgetCache*>::Iterator iter;
  for(iter = widget_cache.begin(); iter != widget_cache.end(); iter++)
    delete *iter;

  widget_cache.clear();
}

void MyCmdWidgetFactory::removeCache(QObject *object)
{
  QWidget *widget = qobject_cast<QWidget *>(object);
  if(widget)
  {
    // Remove the widget cache for the parent widget.
    QMap<QWidget *, NavigationWidgetCache *>::Iterator iter =
        widget_cache.find(widget);
    if(iter != widget_cache.end())
    {
      delete *iter;
      widget_cache.erase(iter);
    }
  }
}

// Create and return a widget based on the marker (used for
// the get_widget_using_cache example).
QWidget* MyCmdWidgetFactory::createWidget(const QString &name)
{
  // We don't worry about setting the widget parent here because it is
  // set in the getWidget() function.

  if(name == "CCXBlocksModify")
    return new CCXBlocksElementTypePanel();
  else if(name == "CCXSectionsCreateSolid")
    return new SectionsCreateSolidPanel();
  else if(name == "CCXSectionsCreateShell")
    return new SectionsCreateShellPanel();
  else if(name == "CCXSectionsCreateBeam")
    return new SectionsCreateBeamPanel();
  else if(name == "CCXSectionsCreateMembrane")
    return new SectionsCreateMembranePanel();
  else if(name == "CCXSectionsModifySolid")
    return new SectionsModifySolidPanel();
  else if(name == "CCXSectionsModifyShell")
    return new SectionsModifyShellPanel();
  else if(name == "CCXSectionsModifyBeam")
    return new SectionsModifyBeamPanel();
  else if(name == "CCXSectionsModifyMembrane")
    return new SectionsModifyMembranePanel();
  else if(name == "CCXSectionsDelete")
    return new SectionsDeletePanel();
  else if(name == "CCXConstraintsCreateRigidBody")
    return new ConstraintsCreateRigidBodyPanel();
  else if(name == "CCXConstraintsCreateTie")
    return new ConstraintsCreateTiePanel();
  else if(name == "CCXConstraintsModifyRigidBody")
    return new ConstraintsModifyRigidBodyPanel();
  else if(name == "CCXConstraintsModifyTie")
    return new ConstraintsModifyTiePanel();
  else if(name == "CCXConstraintsDelete")
    return new ConstraintsDeletePanel();
  else if(name == "CCXSurfaceInteractionsCreate")
    return new SurfaceInteractionsCreatePanel();
  else if(name == "CCXSurfaceInteractionsModify")
    return new SurfaceInteractionsModifyPanel();
  else if(name == "CCXSurfaceInteractionsDelete")
    return new SurfaceInteractionsDeletePanel();
  else if(name == "CCXContactPairsCreate")
    return new ContactPairsCreatePanel();
  else if(name == "CCXContactPairsModify")
    return new ContactPairsModifyPanel();
  else if(name == "CCXContactPairsDelete")
    return new ContactPairsDeletePanel();
  else if(name == "CCXAmplitudesCreate")
    return new AmplitudesCreatePanel();
  else if(name == "CCXAmplitudesModify")
    return new AmplitudesModifyPanel();
  else if(name == "CCXAmplitudesDelete")
    return new AmplitudesDeletePanel();
  else if(name == "CCXLoadsForcesModify")
    return new LoadsForcesModifyPanel();
  else if(name == "CCXLoadsPressuresModify")
    return new LoadsPressuresModifyPanel();
  else if(name == "CCXLoadsHeatfluxesModify")
    return new LoadsHeatfluxesModifyPanel();
  else if(name == "CCXBCsDisplacementsModify")
    return new BCsDisplacementsModifyPanel();
  else if(name == "CCXBCsTemperaturesModify")
    return new BCsTemperaturesModifyPanel();
  else if(name == "CCXHistoryOutputsCreate")
    return new HistoryOutputsCreatePanel();
  else if(name == "CCXHistoryOutputsModifyNode")
    return new HistoryOutputsModifyNodePanel();
  else if(name == "CCXHistoryOutputsModifyElement")
    return new HistoryOutputsModifyElementPanel();
  else if(name == "CCXHistoryOutputsModifyContact")
    return new HistoryOutputsModifyContactPanel();
  else if(name == "CCXHistoryOutputsDelete")
    return new HistoryOutputsDeletePanel();
  else if(name == "CCXFieldOutputsCreate")
    return new FieldOutputsCreatePanel();
  else if(name == "CCXFieldOutputsModifyNode")
    return new FieldOutputsModifyNodePanel();
  else if(name == "CCXFieldOutputsModifyElement")
    return new FieldOutputsModifyElementPanel();
  else if(name == "CCXFieldOutputsModifyContact")
    return new FieldOutputsModifyContactPanel();
  else if(name == "CCXFieldOutputsDelete")
    return new FieldOutputsDeletePanel();
  else if(name == "CCXInitialConditionsCreate")
    return new InitialConditionsCreatePanel();
  else if(name == "CCXInitialConditionsModify")
    return new InitialConditionsModifyPanel();
  else if(name == "CCXInitialConditionsDelete")
    return new InitialConditionsDeletePanel();
  else if(name == "CCXStepsCreate")
    return new StepsCreatePanel();
  else if(name == "CCXStepsModifyParameter")
    return new StepsModifyParameterPanel();
  else if(name == "CCXStepsModifyStatic")
    return new StepsModifyStaticPanel();
  else if(name == "CCXStepsModifyFrequency")
    return new StepsModifyFrequencyPanel();
  else if(name == "CCXStepsModifyBuckle")
    return new StepsModifyBucklePanel();
  else if(name == "CCXStepsModifyHeatTransfer")
    return new StepsModifyHeatTransferPanel();
  else if(name == "CCXStepsModifyCoupledTemperatureDisplacement")
    return new StepsModifyCoupledTemperatureDisplacementPanel();
  else if(name == "CCXStepsModifyUncoupledTemperatureDisplacement")
    return new StepsModifyUncoupledTemperatureDisplacementPanel();
  else if(name == "CCXStepsDelete")
    return new StepsDeletePanel();
  else if(name == "CCXCustomLinesCreate")
    return new CustomLinesCreatePanel();
  else if(name == "CCXCustomLinesModify")
    return new CustomLinesModifyPanel();
  else if(name == "CCXCustomLinesDelete")
    return new CustomLinesDeletePanel();
  else if(name == "CCXJobsCreate")
    return new JobsCreatePanel();
  else if(name == "CCXJobsModify")
    return new JobsModifyPanel();
  else if(name == "CCXJobsDelete")
    return new JobsDeletePanel();
  else if(name == "CCXJobsActions")
    return new JobsActionsPanel();
  else
    return NULL;
}