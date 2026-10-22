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
#include "GUI/mesh/CCXBlocksElementTypePanel.hpp"
#include "GUI/sections/SectionsCreateSolidPanel.hpp"
#include "GUI/sections/SectionsCreateShellPanel.hpp"
#include "GUI/sections/SectionsCreateBeamPanel.hpp"
#include "GUI/sections/SectionsCreateMembranePanel.hpp"
#include "GUI/sections/SectionsModifySolidPanel.hpp"
#include "GUI/sections/SectionsModifyShellPanel.hpp"
#include "GUI/sections/SectionsModifyBeamPanel.hpp"
#include "GUI/sections/SectionsModifyMembranePanel.hpp"
#include "GUI/sections/SectionsDeletePanel.hpp"
#include "GUI/interactions/ConstraintsCreateRigidBodyPanel.hpp"
#include "GUI/interactions/ConstraintsCreateTiePanel.hpp"
#include "GUI/interactions/ConstraintsCreateTieFromCubitContactPairPanel.hpp"
#include "GUI/interactions/ConstraintsCreateEquationPanel.hpp"
#include "GUI/interactions/ConstraintsCreateEquationFromGroupPanel.hpp"
#include "GUI/interactions/ConstraintsModifyRigidBodyPanel.hpp"
#include "GUI/interactions/ConstraintsModifyTiePanel.hpp"
#include "GUI/interactions/ConstraintsModifyEquationPanel.hpp"
#include "GUI/interactions/ConstraintsDeletePanel.hpp"
#include "GUI/interactions/SurfaceInteractionsCreatePanel.hpp"
#include "GUI/interactions/SurfaceInteractionsModifyPanel.hpp"
#include "GUI/interactions/SurfaceInteractionsDeletePanel.hpp"
#include "GUI/interactions/ContactPairsCreatePanel.hpp"
#include "GUI/interactions/ContactPairsCreateFromCubitContactPairPanel.hpp"
#include "GUI/interactions/ContactPairsModifyPanel.hpp"
#include "GUI/interactions/ContactPairsDeletePanel.hpp"
#include "GUI/amplitudes/AmplitudesCreatePanel.hpp"
#include "GUI/amplitudes/AmplitudesModifyPanel.hpp"
#include "GUI/amplitudes/AmplitudesDeletePanel.hpp"
#include "GUI/sections/OrientationsCreatePanel.hpp"
#include "GUI/sections/OrientationsModifyPanel.hpp"
#include "GUI/sections/OrientationsDeletePanel.hpp"
#include "GUI/steps/DampingModifyPanel.hpp"
#include "GUI/steps/DampingDeletePanel.hpp"
#include "GUI/steps/PhysicalConstantsModifyPanel.hpp"
#include "GUI/steps/PhysicalConstantsDeletePanel.hpp"
#include "GUI/loads/LoadsForcesCreatePanel.hpp"
#include "GUI/loads/LoadsForcesDeletePanel.hpp"
#include "GUI/loads/LoadsForcesModifyPanel.hpp"
#include "GUI/loads/LoadsPressuresModifyPanel.hpp"
#include "GUI/loads/LoadsHeatfluxesModifyPanel.hpp"
#include "GUI/loads/LoadsGravityCreatePanel.hpp"
#include "GUI/loads/LoadsGravityModifyPanel.hpp"
#include "GUI/loads/LoadsGravityDeletePanel.hpp"
#include "GUI/loads/LoadsCentrifugalCreatePanel.hpp"
#include "GUI/loads/LoadsCentrifugalModifyPanel.hpp"
#include "GUI/loads/LoadsCentrifugalDeletePanel.hpp"
#include "GUI/loads/LoadsTrajectoryHeatfluxCreatePanel.hpp"
#include "GUI/loads/LoadsTrajectoryHeatfluxModifyPanel.hpp"
#include "GUI/loads/LoadsTrajectoryBodyHeatfluxSphereCreatePanel.hpp"
#include "GUI/loads/LoadsTrajectoryBodyHeatfluxSphereModifyPanel.hpp"
#include "GUI/loads/LoadsTrajectoryDeletePanel.hpp"
#include "GUI/loads/LoadsFilmCreatePanel.hpp"
#include "GUI/loads/LoadsFilmModifyPanel.hpp"
#include "GUI/loads/LoadsFilmDeletePanel.hpp"
#include "GUI/loads/LoadsRadiationCreatePanel.hpp"
#include "GUI/loads/LoadsRadiationModifyPanel.hpp"
#include "GUI/loads/LoadsRadiationDeletePanel.hpp"
#include "GUI/loads/LoadsSurfaceTractionCreatePanel.hpp"
#include "GUI/loads/LoadsSurfaceTractionModifyPanel.hpp"
#include "GUI/loads/LoadsSurfaceTractionDeletePanel.hpp"
#include "GUI/loads/LoadsBodyHeatfluxCreatePanel.hpp"
#include "GUI/loads/LoadsBodyHeatfluxModifyPanel.hpp"
#include "GUI/loads/LoadsBodyHeatfluxDeletePanel.hpp"
#include "GUI/bcs/BCsDisplacementsModifyPanel.hpp"
#include "GUI/bcs/BCsTemperaturesModifyPanel.hpp"
#include "GUI/steps/HistoryOutputsCreatePanel.hpp"
#include "GUI/steps/HistoryOutputsModifyNodePanel.hpp"
#include "GUI/steps/HistoryOutputsModifyElementPanel.hpp"
#include "GUI/steps/HistoryOutputsModifyContactPanel.hpp"
#include "GUI/steps/HistoryOutputsModifySectionPanel.hpp"
#include "GUI/steps/HistoryOutputsDeletePanel.hpp"
#include "GUI/steps/FieldOutputsCreatePanel.hpp"
#include "GUI/steps/FieldOutputsModifyNodePanel.hpp"
#include "GUI/steps/FieldOutputsModifyElementPanel.hpp"
#include "GUI/steps/FieldOutputsModifyContactPanel.hpp"
#include "GUI/steps/FieldOutputsDeletePanel.hpp"
#include "GUI/steps/InitialConditionsCreatePanel.hpp"
#include "GUI/steps/InitialConditionsModifyPanel.hpp"
#include "GUI/steps/InitialConditionsStressAddBlockPanel.hpp"
#include "GUI/steps/InitialConditionsStressAddElementPanel.hpp"
#include "GUI/steps/InitialConditionsDeletePanel.hpp"
#include "GUI/steps/StepsCreatePanel.hpp"
#include "GUI/topology/TopologySettingsPanel.hpp"
#include "GUI/topology/TopologyPreservedSetsPanel.hpp"
#include "GUI/topology/TopologyResultsPanel.hpp"
#include "GUI/topology/TopologyDeletePanel.hpp"
#include "GUI/topology/TopologyActionsPanel.hpp"
#include "GUI/steps/StepsModifyParameterPanel.hpp"
#include "GUI/steps/StepsModifyStaticPanel.hpp"
#include "GUI/steps/StepsModifyFrequencyPanel.hpp"
#include "GUI/steps/StepsModifyBucklePanel.hpp"
#include "GUI/steps/StepsModifyHeatTransferPanel.hpp"
#include "GUI/steps/StepsModifyCoupledTemperatureDisplacementPanel.hpp"
#include "GUI/steps/StepsModifyUncoupledTemperatureDisplacementPanel.hpp"
#include "GUI/steps/StepsModifyDynamicPanel.hpp"
#include "GUI/steps/StepsModifyModalDynamicPanel.hpp"
#include "GUI/steps/StepsModifySteadyStateDynamicsPanel.hpp"
#include "GUI/steps/StepsModifyComplexFrequencyPanel.hpp"
#include "GUI/steps/StepsDeletePanel.hpp"
#include "GUI/export/CustomLinesCreatePanel.hpp"
#include "GUI/export/CustomLinesModifyPanel.hpp"
#include "GUI/export/CustomLinesDeletePanel.hpp"
#include "GUI/jobs/JobsCreatePanel.hpp"
#include "GUI/jobs/JobsModifyPanel.hpp"
#include "GUI/jobs/JobsDeletePanel.hpp"
#include "GUI/jobs/JobsActionsPanel.hpp"

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
  if(name == "CCXConstraintsCreateTieFromCubitContactPair")
    return new ConstraintsCreateTieFromCubitContactPairPanel();
  if(name == "CCXConstraintsCreateEquation")
    return new ConstraintsCreateEquationPanel();
  if(name == "CCXConstraintsCreateEquationFromGroup")
    return new ConstraintsCreateEquationFromGroupPanel();
  if(name == "CCXConstraintsModifyRigidBody")
    return new ConstraintsModifyRigidBodyPanel();
  if(name == "CCXConstraintsModifyTie")
    return new ConstraintsModifyTiePanel();
  if(name == "CCXConstraintsModifyEquation")
    return new ConstraintsModifyEquationPanel();
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
  if(name == "CCXContactPairsCreateFromCubitContactPair")
    return new ContactPairsCreateFromCubitContactPairPanel();
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
  if(name == "CCXLoadsForcesCreate")
  {
    return new LoadsForcesCreatePanel();
  }
  if(name == "CCXLoadsForcesModify")
  {
    return new LoadsForcesModifyPanel();
  }
  if(name == "CCXLoadsForcesDelete")
  {
    return new LoadsForcesDeletePanel();
  }
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
  if(name == "CCXLoadsTrajectoryHeatfluxCreate")
    return new LoadsTrajectoryHeatfluxCreatePanel();
  if(name == "CCXLoadsTrajectoryHeatfluxModify")
    return new LoadsTrajectoryHeatfluxModifyPanel();
  if(name == "CCXLoadsTrajectoryBodyHeatfluxSphereCreate")
    return new LoadsTrajectoryBodyHeatfluxSphereCreatePanel();
  if(name == "CCXLoadsTrajectoryBodyHeatfluxSphereModify")
    return new LoadsTrajectoryBodyHeatfluxSphereModifyPanel();
  if(name == "CCXLoadsTrajectoryDelete")
    return new LoadsTrajectoryDeletePanel();
  if(name == "CCXLoadsFilmCreate")
    return new LoadsFilmCreatePanel();
  if(name == "CCXLoadsFilmModify")
    return new LoadsFilmModifyPanel();
  if(name == "CCXLoadsFilmDelete")
    return new LoadsFilmDeletePanel();
  if(name == "CCXLoadsRadiationCreate")
    return new LoadsRadiationCreatePanel();
  if(name == "CCXLoadsRadiationModify")
    return new LoadsRadiationModifyPanel();
  if(name == "CCXLoadsRadiationDelete")
    return new LoadsRadiationDeletePanel();
  if(name == "CCXLoadsSurfaceTractionCreate")
    return new LoadsSurfaceTractionCreatePanel();
  if(name == "CCXLoadsSurfaceTractionModify")
    return new LoadsSurfaceTractionModifyPanel();
  if(name == "CCXLoadsSurfaceTractionDelete")
    return new LoadsSurfaceTractionDeletePanel();
  if(name == "CCXLoadsBodyHeatfluxCreate")
    return new LoadsBodyHeatfluxCreatePanel();
  if(name == "CCXLoadsBodyHeatfluxModify")
    return new LoadsBodyHeatfluxModifyPanel();
  if(name == "CCXLoadsBodyHeatfluxDelete")
    return new LoadsBodyHeatfluxDeletePanel();
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
  if(name == "CCXHistoryOutputsModifySection")
    return new HistoryOutputsModifySectionPanel();
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
  if(name == "CCXInitialConditionsStressAddBlock")
    return new InitialConditionsStressAddBlockPanel();
  if(name == "CCXInitialConditionsStressAddElement")
    return new InitialConditionsStressAddElementPanel();
  if(name == "CCXInitialConditionsDelete")
    return new InitialConditionsDeletePanel();
  if (name == "CCXTopologyActions")
  {
    return new TopologyActionsPanel();
  }
  if (name == "CCXTopologyDelete")
  {
    return new TopologyDeletePanel();
  }
  if (name == "CCXTopologyResults")
  {
    return new TopologyResultsPanel();
  }
  if (name == "CCXTopologyPreservedSets")
  {
    return new TopologyPreservedSetsPanel();
  }
  if (name == "CCXTopologyCreate")
  {
    return new TopologySettingsPanel(false);
  }
  if (name == "CCXTopologyModify")
  {
    return new TopologySettingsPanel(true);
  }
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
  if(name == "CCXStepsModifyDynamic")
    return new StepsModifyDynamicPanel();
  if(name == "CCXStepsModifyModalDynamic")
    return new StepsModifyModalDynamicPanel();
  if(name == "CCXStepsModifySteadyStateDynamics")
    return new StepsModifySteadyStateDynamicsPanel();
  if(name == "CCXStepsModifyComplexFrequency")
    return new StepsModifyComplexFrequencyPanel();
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
