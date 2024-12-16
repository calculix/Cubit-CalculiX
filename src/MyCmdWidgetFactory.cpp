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
#include "CubitMessage.hpp"

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
#include "OrientationsCreatePanel.hpp"
#include "OrientationsModifyPanel.hpp"
#include "OrientationsDeletePanel.hpp"
#include "DampingModifyPanel.hpp"
#include "DampingDeletePanel.hpp"
#include "PhysicalConstantsModifyPanel.hpp"
#include "PhysicalConstantsDeletePanel.hpp"
#include "LoadsForcesModifyPanel.hpp"
#include "LoadsPressuresModifyPanel.hpp"
#include "LoadsHeatfluxesModifyPanel.hpp"
#include "LoadsGravityCreatePanel.hpp"
#include "LoadsGravityModifyPanel.hpp"
#include "LoadsGravityDeletePanel.hpp"
#include "LoadsCentrifugalCreatePanel.hpp"
#include "LoadsCentrifugalModifyPanel.hpp"
#include "LoadsCentrifugalDeletePanel.hpp"
#include "LoadsTrajectoryCreatePanel.hpp"
#include "LoadsTrajectoryModifyPanel.hpp"
#include "LoadsTrajectoryDeletePanel.hpp"
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

  //std::string output = "need widget " + name.toStdString() + "\n";
  //PRINT_INFO("%s", output.c_str());

  if(name == "CCXBlocksModify")
    return new CCXBlocksElementTypePanel();
  if(name == "CCXSectionsCreateSolid")
    return new SectionsCreateSolidPanel();
  if(name == "CCXSectionsCreateShell")
    return new SectionsCreateShellPanel();
  if(name == "CCXSectionsCreateBeam")
    return new SectionsCreateBeamPanel();
  if(name == "CCXSectionsCreateMembrane")
    return new SectionsCreateMembranePanel();
  if(name == "CCXSectionsModifySolid")
    return new SectionsModifySolidPanel();
  if(name == "CCXSectionsModifyShell")
    return new SectionsModifyShellPanel();
  if(name == "CCXSectionsModifyBeam")
    return new SectionsModifyBeamPanel();
  if(name == "CCXSectionsModifyMembrane")
    return new SectionsModifyMembranePanel();
  if(name == "CCXSectionsDelete")
    return new SectionsDeletePanel();
  if(name == "CCXConstraintsCreateRigidBody")
    return new ConstraintsCreateRigidBodyPanel();
  if(name == "CCXConstraintsCreateTie")
    return new ConstraintsCreateTiePanel();
  if(name == "CCXConstraintsModifyRigidBody")
    return new ConstraintsModifyRigidBodyPanel();
  if(name == "CCXConstraintsModifyTie")
    return new ConstraintsModifyTiePanel();
  if(name == "CCXConstraintsDelete")
    return new ConstraintsDeletePanel();
  if(name == "CCXSurfaceInteractionsCreate")
    return new SurfaceInteractionsCreatePanel();
  if(name == "CCXSurfaceInteractionsModify")
    return new SurfaceInteractionsModifyPanel();
  if(name == "CCXSurfaceInteractionsDelete")
    return new SurfaceInteractionsDeletePanel();
  if(name == "CCXContactPairsCreate")
    return new ContactPairsCreatePanel();
  if(name == "CCXContactPairsModify")
    return new ContactPairsModifyPanel();
  if(name == "CCXContactPairsDelete")
    return new ContactPairsDeletePanel();
  if(name == "CCXAmplitudesCreate")
    return new AmplitudesCreatePanel();
  if(name == "CCXAmplitudesModify")
    return new AmplitudesModifyPanel();
  if(name == "CCXAmplitudesDelete")
    return new AmplitudesDeletePanel();
  if(name == "CCXOrientationsCreate")
    return new OrientationsCreatePanel();
  if(name == "CCXOrientationsModify")
    return new OrientationsModifyPanel();
  if(name == "CCXOrientationsDelete")
    return new OrientationsDeletePanel();
  if(name == "CCXDampingModify")
    return new DampingModifyPanel();
  if(name == "CCXDampingDelete")
    return new DampingDeletePanel();
  if(name == "CCXPhysicalConstantsModify")
    return new PhysicalConstantsModifyPanel();
  if(name == "CCXPhysicalConstantsDelete")
    return new PhysicalConstantsDeletePanel();
  if(name == "CCXLoadsForcesModify")
    return new LoadsForcesModifyPanel();
  if(name == "CCXLoadsPressuresModify")
    return new LoadsPressuresModifyPanel();
  if(name == "CCXLoadsHeatfluxesModify")
    return new LoadsHeatfluxesModifyPanel();
  if(name == "CCXLoadsGravityCreate")
    return new LoadsGravityCreatePanel();
  if(name == "CCXLoadsGravityModify")
    return new LoadsGravityModifyPanel();
  if(name == "CCXLoadsGravityDelete")
    return new LoadsGravityDeletePanel();
  if(name == "CCXLoadsCentrifugalCreate")
    return new LoadsCentrifugalCreatePanel();
  if(name == "CCXLoadsCentrifugalModify")
    return new LoadsCentrifugalModifyPanel();
  if(name == "CCXLoadsCentrifugalDelete")
    return new LoadsCentrifugalDeletePanel();
  if(name == "CCXLoadsTrajectoryCreate")
    return new LoadsTrajectoryCreatePanel();
  if(name == "CCXLoadsTrajectoryModify")
    return new LoadsTrajectoryModifyPanel();
  if(name == "CCXLoadsTrajectoryDelete")
    return new LoadsTrajectoryDeletePanel();
  if(name == "CCXBCsDisplacementsModify")
    return new BCsDisplacementsModifyPanel();
  if(name == "CCXBCsTemperaturesModify")
    return new BCsTemperaturesModifyPanel();
  if(name == "CCXHistoryOutputsCreate")
    return new HistoryOutputsCreatePanel();
  if(name == "CCXHistoryOutputsModifyNode")
    return new HistoryOutputsModifyNodePanel();
  if(name == "CCXHistoryOutputsModifyElement")
    return new HistoryOutputsModifyElementPanel();
  if(name == "CCXHistoryOutputsModifyContact")
    return new HistoryOutputsModifyContactPanel();
  if(name == "CCXHistoryOutputsDelete")
    return new HistoryOutputsDeletePanel();
  if(name == "CCXFieldOutputsCreate")
    return new FieldOutputsCreatePanel();
  if(name == "CCXFieldOutputsModifyNode")
    return new FieldOutputsModifyNodePanel();
  if(name == "CCXFieldOutputsModifyElement")
    return new FieldOutputsModifyElementPanel();
  if(name == "CCXFieldOutputsModifyContact")
    return new FieldOutputsModifyContactPanel();
  if(name == "CCXFieldOutputsDelete")
    return new FieldOutputsDeletePanel();
  if(name == "CCXInitialConditionsCreate")
    return new InitialConditionsCreatePanel();
  if(name == "CCXInitialConditionsModify")
    return new InitialConditionsModifyPanel();
  if(name == "CCXInitialConditionsDelete")
    return new InitialConditionsDeletePanel();
  if(name == "CCXStepsCreate")
    return new StepsCreatePanel();
  if(name == "CCXStepsModifyParameter")
    return new StepsModifyParameterPanel();
  if(name == "CCXStepsModifyStatic")
    return new StepsModifyStaticPanel();
  if(name == "CCXStepsModifyFrequency")
    return new StepsModifyFrequencyPanel();
  if(name == "CCXStepsModifyBuckle")
    return new StepsModifyBucklePanel();
  if(name == "CCXStepsModifyHeatTransfer")
    return new StepsModifyHeatTransferPanel();
  if(name == "CCXStepsModifyCoupledTemperatureDisplacement")
    return new StepsModifyCoupledTemperatureDisplacementPanel();
  if(name == "CCXStepsModifyUncoupledTemperatureDisplacement")
    return new StepsModifyUncoupledTemperatureDisplacementPanel();
  if(name == "CCXStepsDelete")
    return new StepsDeletePanel();
  if(name == "CCXCustomLinesCreate")
    return new CustomLinesCreatePanel();
  if(name == "CCXCustomLinesModify")
    return new CustomLinesModifyPanel();
  if(name == "CCXCustomLinesDelete")
    return new CustomLinesDeletePanel();
  if(name == "CCXJobsCreate")
    return new JobsCreatePanel();
  if(name == "CCXJobsModify")
    return new JobsModifyPanel();
  if(name == "CCXJobsDelete")
    return new JobsDeletePanel();
  if(name == "CCXJobsActions")
    return new JobsActionsPanel();
  
  return NULL;
}