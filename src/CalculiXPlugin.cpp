#include "CalculiXPlugin.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "Core/application/CoreObserver.hpp"

#include "Commands/application/ccxVersionCommand.hpp"
#include "Commands/application/ccxResetCommand.hpp"
#include "Commands/export/ccxExportCommand.hpp"
#include "Commands/application/ccxInitPythonInterfaceCommand.hpp"
#include "Commands/application/ccxUseAutoCleanupCommand.hpp"
#include "Commands/application/ccxUseLogCoreCommand.hpp"
#include "Commands/application/ccxPrintCoreCommand.hpp"
#include "Commands/application/ccxLogCoreCommand.hpp"
#include "Commands/drawing/ccxDrawCommand.hpp"
#include "Commands/drawing/ccxDrawStepCommand.hpp"
#include "Commands/mesh/ccxBlockElementTypeCommand.hpp"
#include "Commands/sections/ccxSectionDeleteCommand.hpp"
#include "Commands/sections/ccxSectionSolidCreateCommand.hpp"
#include "Commands/sections/ccxSectionSolidModifyCommand.hpp"
#include "Commands/sections/ccxSectionShellCreateCommand.hpp"
#include "Commands/sections/ccxSectionShellModifyCommand.hpp"
#include "Commands/sections/ccxSectionBeamCreateCommand.hpp"
#include "Commands/sections/ccxSectionBeamModifyCommand.hpp"
#include "Commands/sections/ccxSectionMembraneCreateCommand.hpp"
#include "Commands/sections/ccxSectionMembraneModifyCommand.hpp"
#include "Commands/interactions/ccxConstraintDeleteCommand.hpp"
#include "Commands/interactions/ccxConstraintEquationCreateCommand.hpp"
#include "Commands/interactions/ccxConstraintEquationCreateFromCoincidentNodesCommand.hpp"
#include "Commands/interactions/ccxConstraintEquationModifyCommand.hpp"
#include "Commands/interactions/ccxConstraintRigidBodyCreateCommand.hpp"
#include "Commands/interactions/ccxConstraintRigidBody2CreateCommand.hpp"
#include "Commands/interactions/ccxConstraintRigidBodyModifyCommand.hpp"
#include "Commands/interactions/ccxConstraintTieCreateCommand.hpp"
#include "Commands/interactions/ccxConstraintTieModifyCommand.hpp"
#include "Commands/interactions/ccxConstraintTieCreateFromCubitContactPairCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionExponentialCreateCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionLinearCreateCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionTabularCreateCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionTiedCreateCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionHardCreateCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionNameModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionExponentialModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionLinearModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionTabularModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionTiedModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionGapConductanceModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionGapHeatGenerationModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionFrictionModifyCommand.hpp"
#include "Commands/interactions/ccxSurfaceInteractionDeleteCommand.hpp"
#include "Commands/interactions/ccxContactPairCreateCommand.hpp"
#include "Commands/interactions/ccxContactPairModifyCommand.hpp"
#include "Commands/interactions/ccxContactPairDeleteCommand.hpp"
#include "Commands/interactions/ccxContactPairCreateFromCubitContactPairCommand.hpp"
#include "Commands/amplitudes/ccxAmplitudeCreateCommand.hpp"
#include "Commands/amplitudes/ccxAmplitudeModifyCommand.hpp"
#include "Commands/amplitudes/ccxAmplitudeDeleteCommand.hpp"
#include "Commands/sections/ccxOrientationCreateCommand.hpp"
#include "Commands/sections/ccxOrientationModifyCommand.hpp"
#include "Commands/sections/ccxOrientationDeleteCommand.hpp"
#include "Commands/steps/ccxDampingModifyCommand.hpp"
#include "Commands/steps/ccxDampingDeleteCommand.hpp"
#include "Commands/steps/ccxPhysicalConstantsModifyCommand.hpp"
#include "Commands/steps/ccxPhysicalConstantsDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsForcesModifyCommand.hpp"
#include "Commands/loads/ccxLoadsPressuresModifyCommand.hpp"
#include "Commands/loads/ccxLoadsHeatfluxesModifyCommand.hpp"
#include "Commands/loads/ccxLoadsGravityCreateCommand.hpp"
#include "Commands/loads/ccxLoadsGravityModifyCommand.hpp"
#include "Commands/loads/ccxLoadsGravityDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsCentrifugalCreateCommand.hpp"
#include "Commands/loads/ccxLoadsCentrifugalModifyCommand.hpp"
#include "Commands/loads/ccxLoadsCentrifugalDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsTrajectoryHeatfluxCreateCommand.hpp"
#include "Commands/loads/ccxLoadsTrajectoryHeatfluxModifyCommand.hpp"
#include "Commands/loads/ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand.hpp"
#include "Commands/loads/ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand.hpp"
#include "Commands/loads/ccxLoadsTrajectoryDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsFilmCreateCommand.hpp"
#include "Commands/loads/ccxLoadsFilmModifyCommand.hpp"
#include "Commands/loads/ccxLoadsFilmDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsRadiationCreateCommand.hpp"
#include "Commands/loads/ccxLoadsRadiationModifyCommand.hpp"
#include "Commands/loads/ccxLoadsRadiationDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsSurfaceTractionCreateCommand.hpp"
#include "Commands/loads/ccxLoadsSurfaceTractionModifyCommand.hpp"
#include "Commands/loads/ccxLoadsSurfaceTractionDeleteCommand.hpp"
#include "Commands/loads/ccxLoadsBodyHeatfluxCreateCommand.hpp"
#include "Commands/loads/ccxLoadsBodyHeatfluxModifyCommand.hpp"
#include "Commands/loads/ccxLoadsBodyHeatfluxDeleteCommand.hpp"
#include "Commands/bcs/ccxBCsDisplacementsModifyCommand.hpp"
#include "Commands/bcs/ccxBCsTemperaturesModifyCommand.hpp"
#include "Commands/steps/ccxHistoryOutputCreateCommand.hpp"
#include "Commands/steps/ccxHistoryOutputNodeModifyCommand.hpp"
#include "Commands/steps/ccxHistoryOutputElementModifyCommand.hpp"
#include "Commands/steps/ccxHistoryOutputContactModifyCommand.hpp"
#include "Commands/steps/ccxHistoryOutputSectionModifyCommand.hpp"
#include "Commands/steps/ccxHistoryOutputDeleteCommand.hpp"
#include "Commands/steps/ccxFieldOutputCreateCommand.hpp"
#include "Commands/steps/ccxFieldOutputNodeModifyCommand.hpp"
#include "Commands/steps/ccxFieldOutputElementModifyCommand.hpp"
#include "Commands/steps/ccxFieldOutputContactModifyCommand.hpp"
#include "Commands/steps/ccxFieldOutputDeleteCommand.hpp"
#include "Commands/steps/ccxInitialConditionCreateCommand.hpp"
#include "Commands/steps/ccxInitialConditionDisplacementModifyCommand.hpp"
#include "Commands/steps/ccxInitialConditionTemperatureModifyCommand.hpp"
#include "Commands/steps/ccxInitialConditionStressAddCommand.hpp"
#include "Commands/steps/ccxInitialConditionDeleteCommand.hpp"
#include "Commands/bcs/ccxHBCAddBCCommand.hpp"
#include "Commands/bcs/ccxHBCRemoveBCCommand.hpp"
#include "Commands/steps/ccxStepCreateCommand.hpp"
#include "Commands/steps/ccxStepAddLoadCommand.hpp"
#include "Commands/steps/ccxStepAddBCCommand.hpp"
#include "Commands/steps/ccxStepAddHistoryOutputCommand.hpp"
#include "Commands/steps/ccxStepAddFieldOutputCommand.hpp"
#include "Commands/steps/ccxStepRemoveLoadCommand.hpp"
#include "Commands/steps/ccxStepRemoveBCCommand.hpp"
#include "Commands/steps/ccxStepRemoveHistoryOutputCommand.hpp"
#include "Commands/steps/ccxStepRemoveFieldOutputCommand.hpp"
#include "Commands/steps/ccxStepParameterModifyCommand.hpp"
#include "Commands/steps/ccxStepStaticModifyCommand.hpp"
#include "Commands/steps/ccxStepFrequencyModifyCommand.hpp"
#include "Commands/steps/ccxStepBuckleModifyCommand.hpp"
#include "Commands/steps/ccxStepHeatTransferModifyCommand.hpp"
#include "Commands/steps/ccxStepCoupledTemperatureDisplacementModifyCommand.hpp"
#include "Commands/steps/ccxStepUncoupledTemperatureDisplacementModifyCommand.hpp"
#include "Commands/steps/ccxStepDynamicModifyCommand.hpp"
#include "Commands/steps/ccxStepModalDynamicModifyCommand.hpp"
#include "Commands/steps/ccxStepSteadyStateDynamicsModifyCommand.hpp"
#include "Commands/steps/ccxStepComplexFrequencyModifyCommand.hpp"
#include "Commands/steps/ccxStepDeleteCommand.hpp"
#include "Commands/steps/ccxStepUtilityModelChangeElementDummyStepCommand.hpp"
#include "Commands/jobs/ccxJobCreateCommand.hpp"
#include "Commands/jobs/ccxJobModifyCommand.hpp"
#include "Commands/jobs/ccxJobDeleteCommand.hpp"
#include "Commands/jobs/ccxRunJobCommand.hpp"
#include "Commands/topology/ccxTopologyCreateCommand.hpp"
#include "Commands/topology/ccxTopologyModifyCommand.hpp"
#include "Commands/topology/ccxTopologyDeleteCommand.hpp"
#include "Commands/topology/ccxTopologyAddSetsCommand.hpp"
#include "Commands/topology/ccxTopologyRemoveSetsCommand.hpp"
#include "Commands/topology/ccxTopologyResetPreservedCommand.hpp"
#include "Commands/topology/ccxRunTopologyCommand.hpp"
#include "Commands/topology/ccxWaitTopologyCommand.hpp"
#include "Commands/topology/ccxKillTopologyCommand.hpp"
#include "Commands/jobs/ccxWaitJobCommand.hpp"
#include "Commands/results/ccxResultCGXJobCommand.hpp"
#include "Commands/results/ccxResultParaViewJobCommand.hpp"
#include "Commands/results/ccxResultLoadJobCommand.hpp"
#include "Commands/results/ccxResultConvertJobCommand.hpp"
#include "Commands/results/frd/ccxResultPlotJobFrdCommand.hpp"
#include "Commands/results/dat/ccxResultPlotJobDatCommand.hpp"
#include "Commands/results/dat/ccxResultPlotJobDatSectionCommand.hpp"
#include "Commands/results/frd/ccxResultCSVJobFrdCommand.hpp"
#include "Commands/results/dat/ccxResultCSVJobDatCommand.hpp"
#include "Commands/results/dat/ccxResultCSVJobDatSectionCommand.hpp"
#include "Commands/results/ccxResultProjectJobCommand.hpp"
#include "Commands/jobs/ccxKillJobCommand.hpp"
#include "Commands/export/ccxCustomLineCreateCommand.hpp"
#include "Commands/export/ccxCustomLineModifyCommand.hpp"
#include "Commands/export/ccxCustomLineDeleteCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryCreateGroupCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryModifyGroupCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryDeleteGroupCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryCreateMaterialCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryModifyMaterialCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryDeleteMaterialCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryExportMaterialCommand.hpp"
#include "Commands/materials/ccxMaterialLibraryImportMaterialCommand.hpp"
#include "Commands/mesh/ccxWrapCommand.hpp"
#include "Commands/drawing/ccxAnimateTopologyCommand.hpp"

//! This macro is required to identify this as a valid Cubit plugin. The plugin
//! will NOT be loaded if this macro is not present.
CUBIT_PLUGIN(CalculiXPlugin)

CalculiXPlugin::CalculiXPlugin()
{
  CalculiXApplicationInterface ccx_application_iface;

  ccx_core.init();
  ccx_core.init2(); // to init missing material core and interfaces
  ccx_application_iface.core_observer().register_observer();
  //ccx_core.init_pythoninterface(); // can't start up with the plugin, needs to be started with the component
  //ccx_core.init_materiallibrary();
}

CalculiXPlugin::~CalculiXPlugin()
{
  CalculiXApplicationInterface ccx_application_iface;

  ccx_application_iface.core_observer().unregister_observer();
}

std::vector<std::string> CalculiXPlugin::get_keys()
{
  std::vector<std::string> keys;
  keys.push_back("ccxVersionCommand");
  keys.push_back("ccxResetCommand");
  keys.push_back("ccxExportCommand");
  keys.push_back("ccxInitPythonInterfaceCommand");
  keys.push_back("ccxUseAutoCleanupCommand");
  keys.push_back("ccxUseLogCoreCommand");
  keys.push_back("ccxPrintCoreCommand");
  keys.push_back("ccxLogCoreCommand");
  keys.push_back("ccxDrawCommand");
  keys.push_back("ccxDrawStepCommand");
  keys.push_back("ccxBlockElementTypeCommand");
  keys.push_back("ccxSectionDeleteCommand");
  keys.push_back("ccxSectionSolidCreateCommand");
  keys.push_back("ccxSectionSolidModifyCommand");
  keys.push_back("ccxSectionShellCreateCommand");
  keys.push_back("ccxSectionShellModifyCommand");
  keys.push_back("ccxSectionBeamCreateCommand");
  keys.push_back("ccxSectionBeamModifyCommand");
  keys.push_back("ccxSectionMembraneCreateCommand");
  keys.push_back("ccxSectionMembraneModifyCommand");
  keys.push_back("ccxConstraintDeleteCommand");
  keys.push_back("ccxConstraintEquationCreateCommand");
  keys.push_back("ccxConstraintEquationCreateFromCoincidentNodesCommand");
  keys.push_back("ccxConstraintEquationModifyCommand");
  keys.push_back("ccxConstraintRigidBodyCreateCommand");
  keys.push_back("ccxConstraintRigidBody2CreateCommand");
  keys.push_back("ccxConstraintRigidBodyModifyCommand");
  keys.push_back("ccxConstraintTieCreateCommand");
  keys.push_back("ccxConstraintTieModifyCommand");
  keys.push_back("ccxConstraintTieCreateFromCubitContactPairCommand");
  keys.push_back("ccxSurfaceInteractionExponentialCreateCommand");
  keys.push_back("ccxSurfaceInteractionLinearCreateCommand");
  keys.push_back("ccxSurfaceInteractionTabularCreateCommand");
  keys.push_back("ccxSurfaceInteractionTiedCreateCommand");
  keys.push_back("ccxSurfaceInteractionHardCreateCommand");
  keys.push_back("ccxSurfaceInteractionNameModifyCommand");
  keys.push_back("ccxSurfaceInteractionExponentialModifyCommand");
  keys.push_back("ccxSurfaceInteractionLinearModifyCommand");
  keys.push_back("ccxSurfaceInteractionTabularModifyCommand");
  keys.push_back("ccxSurfaceInteractionTiedModifyCommand");
  keys.push_back("ccxSurfaceInteractionGapConductanceModifyCommand");
  keys.push_back("ccxSurfaceInteractionGapHeatGenerationModifyCommand");
  keys.push_back("ccxSurfaceInteractionFrictionModifyCommand");
  keys.push_back("ccxSurfaceInteractionDeleteCommand");
  keys.push_back("ccxContactPairCreateCommand");
  keys.push_back("ccxContactPairModifyCommand");
  keys.push_back("ccxContactPairDeleteCommand");
  keys.push_back("ccxContactPairCreateFromCubitContactPairCommand");
  keys.push_back("ccxAmplitudeCreateCommand");
  keys.push_back("ccxAmplitudeModifyCommand");
  keys.push_back("ccxAmplitudeDeleteCommand");
  keys.push_back("ccxOrientationCreateCommand");
  keys.push_back("ccxOrientationModifyCommand");
  keys.push_back("ccxOrientationDeleteCommand");
  keys.push_back("ccxDampingModifyCommand");
  keys.push_back("ccxDampingDeleteCommand");
  keys.push_back("ccxPhysicalConstantsModifyCommand");
  keys.push_back("ccxPhysicalConstantsDeleteCommand");
  keys.push_back("ccxLoadsForcesModifyCommand");
  keys.push_back("ccxLoadsPressuresModifyCommand");
  keys.push_back("ccxLoadsHeatfluxesModifyCommand");
  keys.push_back("ccxLoadsGravityCreateCommand");
  keys.push_back("ccxLoadsGravityModifyCommand");
  keys.push_back("ccxLoadsGravityDeleteCommand");
  keys.push_back("ccxLoadsCentrifugalCreateCommand");
  keys.push_back("ccxLoadsCentrifugalModifyCommand");
  keys.push_back("ccxLoadsCentrifugalDeleteCommand");
  keys.push_back("ccxLoadsTrajectoryHeatfluxCreateCommand");
  keys.push_back("ccxLoadsTrajectoryHeatfluxModifyCommand");
  keys.push_back("ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand");
  keys.push_back("ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand");
  keys.push_back("ccxLoadsTrajectoryDeleteCommand");
  keys.push_back("ccxLoadsFilmCreateCommand");
  keys.push_back("ccxLoadsFilmModifyCommand");
  keys.push_back("ccxLoadsFilmDeleteCommand");
  keys.push_back("ccxLoadsRadiationCreateCommand");
  keys.push_back("ccxLoadsRadiationModifyCommand");
  keys.push_back("ccxLoadsRadiationDeleteCommand");
  keys.push_back("ccxLoadsSurfaceTractionCreateCommand");
  keys.push_back("ccxLoadsSurfaceTractionModifyCommand");
  keys.push_back("ccxLoadsSurfaceTractionDeleteCommand");
  keys.push_back("ccxLoadsBodyHeatfluxCreateCommand");
  keys.push_back("ccxLoadsBodyHeatfluxModifyCommand");
  keys.push_back("ccxLoadsBodyHeatfluxDeleteCommand");
  keys.push_back("ccxBCsDisplacementsModifyCommand");
  keys.push_back("ccxBCsTemperaturesModifyCommand");
  keys.push_back("ccxHistoryOutputCreateCommand");
  keys.push_back("ccxHistoryOutputNodeModifyCommand");
  keys.push_back("ccxHistoryOutputElementModifyCommand");
  keys.push_back("ccxHistoryOutputContactModifyCommand");
  keys.push_back("ccxHistoryOutputSectionModifyCommand");
  keys.push_back("ccxHistoryOutputDeleteCommand");
  keys.push_back("ccxFieldOutputCreateCommand");
  keys.push_back("ccxFieldOutputNodeModifyCommand");
  keys.push_back("ccxFieldOutputElementModifyCommand");
  keys.push_back("ccxFieldOutputContactModifyCommand");
  keys.push_back("ccxFieldOutputDeleteCommand");
  keys.push_back("ccxInitialConditionCreateCommand");
  keys.push_back("ccxInitialConditionDisplacementModifyCommand");
  keys.push_back("ccxInitialConditionTemperatureModifyCommand");
  keys.push_back("ccxInitialConditionStressAddCommand");
  keys.push_back("ccxInitialConditionDeleteCommand");
  keys.push_back("ccxHBCAddBCCommand");
  keys.push_back("ccxHBCRemoveBCCommand");
  keys.push_back("ccxStepCreateCommand");
  keys.push_back("ccxStepAddLoadCommand");
  keys.push_back("ccxStepAddBCCommand");
  keys.push_back("ccxStepAddHistoryOutputCommand");
  keys.push_back("ccxStepAddFieldOutputCommand");
  keys.push_back("ccxStepRemoveLoadCommand");
  keys.push_back("ccxStepRemoveBCCommand");
  keys.push_back("ccxStepRemoveHistoryOutputCommand");
  keys.push_back("ccxStepRemoveFieldOutputCommand");
  keys.push_back("ccxStepParameterModifyCommand");
  keys.push_back("ccxStepStaticModifyCommand");
  keys.push_back("ccxStepFrequencyModifyCommand");
  keys.push_back("ccxStepBuckleModifyCommand");
  keys.push_back("ccxStepHeatTransferModifyCommand");
  keys.push_back("ccxStepCoupledTemperatureDisplacementModifyCommand");
  keys.push_back("ccxStepUncoupledTemperatureDisplacementModifyCommand");
  keys.push_back("ccxStepDynamicModifyCommand");
  keys.push_back("ccxStepModalDynamicModifyCommand");
  keys.push_back("ccxStepSteadyStateDynamicsModifyCommand");
  keys.push_back("ccxStepComplexFrequencyModifyCommand");
  keys.push_back("ccxStepDeleteCommand");
  keys.push_back("ccxStepUtilityModelChangeElementDummyStepCommand");
  keys.push_back("ccxJobCreateCommand");
  keys.push_back("ccxJobModifyCommand");
  keys.push_back("ccxJobDeleteCommand");
  keys.push_back("ccxRunJobCommand");
  keys.push_back("ccxTopologyCreateCommand");
  keys.push_back("ccxTopologyModifyCommand");
  keys.push_back("ccxTopologyDeleteCommand");
  keys.push_back("ccxTopologyAddSetsCommand");
  keys.push_back("ccxTopologyRemoveSetsCommand");
  keys.push_back("ccxTopologyResetPreservedCommand");
  keys.push_back("ccxRunTopologyCommand");
  keys.push_back("ccxWaitTopologyCommand");
  keys.push_back("ccxKillTopologyCommand");
  keys.push_back("ccxWaitJobCommand");
  keys.push_back("ccxKillJobCommand");
  keys.push_back("ccxResultCGXJobCommand");
  keys.push_back("ccxResultParaViewJobCommand");
  keys.push_back("ccxResultLoadJobCommand");
  keys.push_back("ccxResultConvertJobCommand");
  keys.push_back("ccxResultPlotJobFrdCommand");
  keys.push_back("ccxResultPlotJobDatCommand");
  keys.push_back("ccxResultPlotJobDatSectionCommand");
  keys.push_back("ccxResultCSVJobFrdCommand");
  keys.push_back("ccxResultCSVJobDatCommand");
  keys.push_back("ccxResultCSVJobDatSectionCommand");
  keys.push_back("ccxResultProjectJobCommand");
  keys.push_back("ccxCustomLineCreateCommand");
  keys.push_back("ccxCustomLineModifyCommand");
  keys.push_back("ccxCustomLineDeleteCommand");
  keys.push_back("ccxMaterialLibraryCreateGroupCommand");
  keys.push_back("ccxMaterialLibraryModifyGroupCommand");
  keys.push_back("ccxMaterialLibraryDeleteGroupCommand");
  keys.push_back("ccxMaterialLibraryCreateMaterialCommand");
  keys.push_back("ccxMaterialLibraryModifyMaterialCommand");
  keys.push_back("ccxMaterialLibraryDeleteMaterialCommand");
  keys.push_back("ccxMaterialLibraryExportMaterialCommand");
  keys.push_back("ccxMaterialLibraryImportMaterialCommand");
  keys.push_back("ccxWrapCommand");
  keys.push_back("ccxAnimateTopologyCommand");

  return keys;
}

CubitCommand* CalculiXPlugin::create_command(const std::string &key)
{
  // NOTE: The internals of Cubit will take owernship of the command,
  // and delete it when it is time to clean up.

  if(key == "ccxVersionCommand")
  {
    return new ccxVersionCommand();
  }
  
  if(key == "ccxResetCommand")
  {
    return new ccxResetCommand();
  }

  if(key == "ccxExportCommand")
  {
    return new ccxExportCommand();
  }

  if(key == "ccxUseAutoCleanupCommand")
  {
    return new ccxUseAutoCleanupCommand();
  }
  
  if(key == "ccxInitPythonInterfaceCommand")
  {
    return new ccxInitPythonInterfaceCommand();
  }

  if(key == "ccxUseLogCoreCommand")
  {
    return new ccxUseLogCoreCommand();
  }

  if(key == "ccxPrintCoreCommand")
  {
    return new ccxPrintCoreCommand();
  }
  
  if(key == "ccxLogCoreCommand")
  {
    return new ccxLogCoreCommand();
  }
  
  if(key == "ccxDrawCommand")
  {
    return new ccxDrawCommand();
  }
  
  if(key == "ccxDrawStepCommand")
  {
    return new ccxDrawStepCommand();
  }

  if(key == "ccxBlockElementTypeCommand")
  {
    return new ccxBlockElementTypeCommand();
  }

  if(key == "ccxSectionDeleteCommand")
  {
    return new ccxSectionDeleteCommand();
  }

  if(key == "ccxSectionSolidCreateCommand")
  {
    return new ccxSectionSolidCreateCommand();
  }

  if(key == "ccxSectionSolidModifyCommand")
  {
    return new ccxSectionSolidModifyCommand();
  }

  if(key == "ccxSectionShellCreateCommand")
  {
    return new ccxSectionShellCreateCommand();
  }

  if(key == "ccxSectionShellModifyCommand")
  {
    return new ccxSectionShellModifyCommand();
  }

  if(key == "ccxSectionBeamCreateCommand")
  {
    return new ccxSectionBeamCreateCommand();
  }

  if(key == "ccxSectionBeamModifyCommand")
  {
    return new ccxSectionBeamModifyCommand();
  }

  if(key == "ccxSectionMembraneCreateCommand")
  {
    return new ccxSectionMembraneCreateCommand();
  }

  if(key == "ccxSectionMembraneModifyCommand")
  {
    return new ccxSectionMembraneModifyCommand();
  }

  if(key == "ccxConstraintDeleteCommand")
  {
    return new ccxConstraintDeleteCommand();
  }

  if(key == "ccxConstraintRigidBodyCreateCommand")
  {
    return new ccxConstraintRigidBodyCreateCommand();
  }

  if(key == "ccxConstraintRigidBody2CreateCommand")
  {
    return new ccxConstraintRigidBody2CreateCommand();
  }
  
  if(key == "ccxConstraintRigidBodyModifyCommand")
  {
    return new ccxConstraintRigidBodyModifyCommand();
  }

  if(key == "ccxConstraintTieCreateCommand")
  {
    return new ccxConstraintTieCreateCommand();
  }

  if(key == "ccxConstraintTieModifyCommand")
  {
    return new ccxConstraintTieModifyCommand();
  }

  if(key == "ccxConstraintEquationCreateCommand")
  {
    return new ccxConstraintEquationCreateCommand();
  }

  if(key == "ccxConstraintEquationCreateFromCoincidentNodesCommand")
  {
    return new ccxConstraintEquationCreateFromCoincidentNodesCommand();
  }
  
  if(key == "ccxConstraintEquationModifyCommand")
  {
    return new ccxConstraintEquationModifyCommand();
  }

  if(key == "ccxConstraintTieCreateFromCubitContactPairCommand")
  {
    return new ccxConstraintTieCreateFromCubitContactPairCommand();
  }

  if(key == "ccxSurfaceInteractionExponentialCreateCommand")
  {
    return new ccxSurfaceInteractionExponentialCreateCommand();
  }

  if(key == "ccxSurfaceInteractionLinearCreateCommand")
  {
    return new ccxSurfaceInteractionLinearCreateCommand();
  }
  
  if(key == "ccxSurfaceInteractionTabularCreateCommand")
  {
    return new ccxSurfaceInteractionTabularCreateCommand();
  }

  if(key == "ccxSurfaceInteractionTiedCreateCommand")
  {
    return new ccxSurfaceInteractionTiedCreateCommand();
  }

  if(key == "ccxSurfaceInteractionHardCreateCommand")
  {
    return new ccxSurfaceInteractionHardCreateCommand();
  }

  if(key == "ccxSurfaceInteractionNameModifyCommand")
  {
    return new ccxSurfaceInteractionNameModifyCommand();
  }

  if(key == "ccxSurfaceInteractionExponentialModifyCommand")
  {
    return new ccxSurfaceInteractionExponentialModifyCommand();
  }

  if(key == "ccxSurfaceInteractionLinearModifyCommand")
  {
    return new ccxSurfaceInteractionLinearModifyCommand();
  }

  if(key == "ccxSurfaceInteractionTabularModifyCommand")
  {
    return new ccxSurfaceInteractionTabularModifyCommand();
  }

  if(key == "ccxSurfaceInteractionTiedModifyCommand")
  {
    return new ccxSurfaceInteractionTiedModifyCommand();
  }

  if(key == "ccxSurfaceInteractionGapConductanceModifyCommand")
  {
    return new ccxSurfaceInteractionGapConductanceModifyCommand();
  }

  if(key == "ccxSurfaceInteractionGapHeatGenerationModifyCommand")
  {
    return new ccxSurfaceInteractionGapHeatGenerationModifyCommand();
  }

  if(key == "ccxSurfaceInteractionFrictionModifyCommand")
  {
    return new ccxSurfaceInteractionFrictionModifyCommand();
  }

  if(key == "ccxSurfaceInteractionDeleteCommand")
  {
    return new ccxSurfaceInteractionDeleteCommand();
  }

  if(key == "ccxContactPairCreateCommand")
  {
    return new ccxContactPairCreateCommand();
  }

  if(key == "ccxContactPairModifyCommand")
  {
    return new ccxContactPairModifyCommand();
  }
  
  if(key == "ccxContactPairDeleteCommand")
  {
    return new ccxContactPairDeleteCommand();
  }
  
  if(key == "ccxContactPairCreateFromCubitContactPairCommand")
  {
    return new ccxContactPairCreateFromCubitContactPairCommand();
  }

  if(key == "ccxAmplitudeCreateCommand")
  {
    return new ccxAmplitudeCreateCommand();
  }

  if(key == "ccxAmplitudeModifyCommand")
  {
    return new ccxAmplitudeModifyCommand();
  }

  if(key == "ccxAmplitudeDeleteCommand")
  {
    return new ccxAmplitudeDeleteCommand();
  }
  
  if(key == "ccxOrientationCreateCommand")
  {
    return new ccxOrientationCreateCommand();
  }

  if(key == "ccxOrientationModifyCommand")
  {
    return new ccxOrientationModifyCommand();
  }

  if(key == "ccxOrientationDeleteCommand")
  {
    return new ccxOrientationDeleteCommand();
  }

  if(key == "ccxDampingModifyCommand")
  {
    return new ccxDampingModifyCommand();
  }

  if(key == "ccxDampingDeleteCommand")
  {
    return new ccxDampingDeleteCommand();
  }

  if(key == "ccxPhysicalConstantsModifyCommand")
  {
    return new ccxPhysicalConstantsModifyCommand();
  }

  if(key == "ccxPhysicalConstantsDeleteCommand")
  {
    return new ccxPhysicalConstantsDeleteCommand();
  }

  if(key == "ccxLoadsForcesModifyCommand")
  {
    return new ccxLoadsForcesModifyCommand();
  }

  if(key == "ccxLoadsPressuresModifyCommand")
  {
    return new ccxLoadsPressuresModifyCommand();
  }

  if(key == "ccxLoadsHeatfluxesModifyCommand")
  {
    return new ccxLoadsHeatfluxesModifyCommand();
  }

  if(key == "ccxLoadsGravityCreateCommand")
  {
    return new ccxLoadsGravityCreateCommand();
  }
  
  if(key == "ccxLoadsGravityModifyCommand")
  {
    return new ccxLoadsGravityModifyCommand();
  }

  if(key == "ccxLoadsGravityDeleteCommand")
  {
    return new ccxLoadsGravityDeleteCommand();
  }

  if(key == "ccxLoadsCentrifugalCreateCommand")
  {
    return new ccxLoadsCentrifugalCreateCommand();
  }
  
  if(key == "ccxLoadsCentrifugalModifyCommand")
  {
    return new ccxLoadsCentrifugalModifyCommand();
  }

  if(key == "ccxLoadsCentrifugalDeleteCommand")
  {
    return new ccxLoadsCentrifugalDeleteCommand();
  }

  if(key == "ccxLoadsTrajectoryHeatfluxCreateCommand")
  {
    return new ccxLoadsTrajectoryHeatfluxCreateCommand();
  }
  
  if(key == "ccxLoadsTrajectoryHeatfluxModifyCommand")
  {
    return new ccxLoadsTrajectoryHeatfluxModifyCommand();
  }
  
  if(key == "ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand")
  {
    return new ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand();
  }

  if(key == "ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand")
  {
    return new ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand();
  }

  if(key == "ccxLoadsTrajectoryDeleteCommand")
  {
    return new ccxLoadsTrajectoryDeleteCommand();
  }

  if(key == "ccxLoadsFilmCreateCommand")
  {
    return new ccxLoadsFilmCreateCommand();
  }

  if(key == "ccxLoadsFilmModifyCommand")
  {
    return new ccxLoadsFilmModifyCommand();
  }

  if(key == "ccxLoadsFilmDeleteCommand")
  {
    return new ccxLoadsFilmDeleteCommand();
  }

  if(key == "ccxLoadsRadiationCreateCommand")
  {
    return new ccxLoadsRadiationCreateCommand();
  }

  if(key == "ccxLoadsRadiationModifyCommand")
  {
    return new ccxLoadsRadiationModifyCommand();
  }

  if(key == "ccxLoadsRadiationDeleteCommand")
  {
    return new ccxLoadsRadiationDeleteCommand();
  }

  if(key == "ccxLoadsSurfaceTractionCreateCommand")
  {
    return new ccxLoadsSurfaceTractionCreateCommand();
  }

  if(key == "ccxLoadsSurfaceTractionModifyCommand")
  {
    return new ccxLoadsSurfaceTractionModifyCommand();
  }

  if(key == "ccxLoadsSurfaceTractionDeleteCommand")
  {
    return new ccxLoadsSurfaceTractionDeleteCommand();
  }
  
  if(key == "ccxLoadsBodyHeatfluxCreateCommand")
  {
    return new ccxLoadsBodyHeatfluxCreateCommand();
  }

  if(key == "ccxLoadsBodyHeatfluxModifyCommand")
  {
    return new ccxLoadsBodyHeatfluxModifyCommand();
  }

  if(key == "ccxLoadsBodyHeatfluxDeleteCommand")
  {
    return new ccxLoadsBodyHeatfluxDeleteCommand();
  }
    
  if(key == "ccxBCsDisplacementsModifyCommand")
  {
    return new ccxBCsDisplacementsModifyCommand();
  }

  if(key == "ccxBCsTemperaturesModifyCommand")
  {
    return new ccxBCsTemperaturesModifyCommand();
  }
  
  if(key == "ccxHistoryOutputCreateCommand")
  {
    return new ccxHistoryOutputCreateCommand();
  }

  if(key == "ccxHistoryOutputNodeModifyCommand")
  {
    return new ccxHistoryOutputNodeModifyCommand();
  }
  
  if(key == "ccxHistoryOutputElementModifyCommand")
  {
    return new ccxHistoryOutputElementModifyCommand();
  }
  
  if(key == "ccxHistoryOutputContactModifyCommand")
  {
    return new ccxHistoryOutputContactModifyCommand();
  }

  if(key == "ccxHistoryOutputSectionModifyCommand")
  {
    return new ccxHistoryOutputSectionModifyCommand();
  }

  if(key == "ccxHistoryOutputDeleteCommand")
  {
    return new ccxHistoryOutputDeleteCommand();
  }

  if(key == "ccxFieldOutputCreateCommand")
  {
    return new ccxFieldOutputCreateCommand();
  }

  if(key == "ccxFieldOutputNodeModifyCommand")
  {
    return new ccxFieldOutputNodeModifyCommand();
  }
  
  if(key == "ccxFieldOutputElementModifyCommand")
  {
    return new ccxFieldOutputElementModifyCommand();
  }

  if(key == "ccxFieldOutputContactModifyCommand")
  {
    return new ccxFieldOutputContactModifyCommand();
  }

  if(key == "ccxFieldOutputDeleteCommand")
  {
    return new ccxFieldOutputDeleteCommand();
  }

  if(key == "ccxInitialConditionCreateCommand")
  {
    return new ccxInitialConditionCreateCommand();
  }

  if(key == "ccxInitialConditionDisplacementModifyCommand")
  {
    return new ccxInitialConditionDisplacementModifyCommand();
  }

  if(key == "ccxInitialConditionTemperatureModifyCommand")
  {
    return new ccxInitialConditionTemperatureModifyCommand();
  }

  if(key == "ccxInitialConditionStressAddCommand")
  {
    return new ccxInitialConditionStressAddCommand();
  }

  if(key == "ccxInitialConditionDeleteCommand")
  {
    return new ccxInitialConditionDeleteCommand();
  }

  if(key == "ccxHBCAddBCCommand")
  {
    return new ccxHBCAddBCCommand();
  }

  if(key == "ccxHBCRemoveBCCommand")
  {
    return new ccxHBCRemoveBCCommand();
  }

  if(key == "ccxStepCreateCommand")
  {
    return new ccxStepCreateCommand();
  }
  
  if(key == "ccxStepAddLoadCommand")
  {
    return new ccxStepAddLoadCommand();
  }

  if(key == "ccxStepAddBCCommand")
  {
    return new ccxStepAddBCCommand();
  }

  if(key == "ccxStepAddHistoryOutputCommand")
  {
    return new ccxStepAddHistoryOutputCommand();
  }
  
  if(key == "ccxStepAddFieldOutputCommand")
  {
    return new ccxStepAddFieldOutputCommand();
  }

  if(key == "ccxStepRemoveLoadCommand")
  {
    return new ccxStepRemoveLoadCommand();
  }

  if(key == "ccxStepRemoveBCCommand")
  {
    return new ccxStepRemoveBCCommand();
  }

  if(key == "ccxStepRemoveHistoryOutputCommand")
  {
    return new ccxStepRemoveHistoryOutputCommand();
  }
  
  if(key == "ccxStepRemoveFieldOutputCommand")
  {
    return new ccxStepRemoveFieldOutputCommand();
  }
  
  if(key == "ccxStepParameterModifyCommand")
  {
    return new ccxStepParameterModifyCommand();
  }

  if(key == "ccxStepStaticModifyCommand")
  {
    return new ccxStepStaticModifyCommand();
  }

  if(key == "ccxStepFrequencyModifyCommand")
  {
    return new ccxStepFrequencyModifyCommand();
  }

  if(key == "ccxStepBuckleModifyCommand")
  {
    return new ccxStepBuckleModifyCommand();
  }

  if(key == "ccxStepHeatTransferModifyCommand")
  {
    return new ccxStepHeatTransferModifyCommand();
  }

  if(key == "ccxStepCoupledTemperatureDisplacementModifyCommand")
  {
    return new ccxStepCoupledTemperatureDisplacementModifyCommand();
  }

  if(key == "ccxStepUncoupledTemperatureDisplacementModifyCommand")
  {
    return new ccxStepUncoupledTemperatureDisplacementModifyCommand();
  }

  if(key == "ccxStepDynamicModifyCommand")
  {
    return new ccxStepDynamicModifyCommand();
  }

  if(key == "ccxStepModalDynamicModifyCommand")
  {
    return new ccxStepModalDynamicModifyCommand();
  }

  if(key == "ccxStepSteadyStateDynamicsModifyCommand")
  {
    return new ccxStepSteadyStateDynamicsModifyCommand();
  }

  if(key == "ccxStepComplexFrequencyModifyCommand")
  {
    return new ccxStepComplexFrequencyModifyCommand();
  }

  if(key == "ccxStepDeleteCommand")
  {
    return new ccxStepDeleteCommand();
  }
  
  if(key == "ccxStepUtilityModelChangeElementDummyStepCommand")
  {
    return new ccxStepUtilityModelChangeElementDummyStepCommand();
  }

  if(key == "ccxJobCreateCommand")
  {
    return new ccxJobCreateCommand();
  }

  if(key == "ccxJobModifyCommand")
  {
    return new ccxJobModifyCommand();
  }
  
  if(key == "ccxJobDeleteCommand")
  {
    return new ccxJobDeleteCommand();
  }

  if(key == "ccxRunJobCommand")
  {
    return new ccxRunJobCommand();
  }
  
  if(key == "ccxWaitJobCommand")
  {
    return new ccxWaitJobCommand();
  }

  if(key == "ccxTopologyCreateCommand")
  {
    return new ccxTopologyCreateCommand();
  }

  if(key == "ccxTopologyModifyCommand")
  {
    return new ccxTopologyModifyCommand();
  }
    
  if(key == "ccxTopologyDeleteCommand")
  {
    return new ccxTopologyDeleteCommand();
  }

  if(key == "ccxTopologyAddSetsCommand")
  {
    return new ccxTopologyAddSetsCommand();
  }

  if(key == "ccxTopologyRemoveSetsCommand")
  {
    return new ccxTopologyRemoveSetsCommand();
  }

  if (key == "ccxTopologyResetPreservedCommand")
  {
    return new ccxTopologyResetPreservedCommand();
  }

  if (key == "ccxWaitTopologyCommand")
  {
    return new ccxWaitTopologyCommand();
  }
  if (key == "ccxKillTopologyCommand")
  {
    return new ccxKillTopologyCommand();
  }

  if(key == "ccxRunTopologyCommand")
  {
    return new ccxRunTopologyCommand();
  }

  if(key == "ccxResultCGXJobCommand")
  {
    return new ccxResultCGXJobCommand();
  }

  if(key == "ccxResultParaViewJobCommand")
  {
    return new ccxResultParaViewJobCommand();
  }

  if(key == "ccxResultLoadJobCommand")
  {
    return new ccxResultLoadJobCommand();
  }

  if(key == "ccxResultConvertJobCommand")
  {
    return new ccxResultConvertJobCommand();
  }

  if(key == "ccxResultProjectJobCommand")
  {
    return new ccxResultProjectJobCommand();
  }

  if(key == "ccxResultPlotJobFrdCommand")
  {
    return new ccxResultPlotJobFrdCommand();
  }

  if(key == "ccxResultPlotJobDatCommand")
  {
    return new ccxResultPlotJobDatCommand();
  }
  
  if(key == "ccxResultPlotJobDatSectionCommand")
  {
    return new ccxResultPlotJobDatSectionCommand();
  }

  if(key == "ccxResultCSVJobFrdCommand")
  {
    return new ccxResultCSVJobFrdCommand();
  }

  if(key == "ccxResultCSVJobDatCommand")
  {
    return new ccxResultCSVJobDatCommand();
  }

  if(key == "ccxResultCSVJobDatSectionCommand")
  {
    return new ccxResultCSVJobDatSectionCommand();
  }

  if(key == "ccxKillJobCommand")
  {
    return new ccxKillJobCommand();
  }

  if(key == "ccxCustomLineCreateCommand")
  {
    return new ccxCustomLineCreateCommand();
  }

  if(key == "ccxCustomLineModifyCommand")
  {
    return new ccxCustomLineModifyCommand();
  }
  
  if(key == "ccxCustomLineDeleteCommand")
  {
    return new ccxCustomLineDeleteCommand();
  }

  if(key == "ccxMaterialLibraryCreateGroupCommand")
  {
    return new ccxMaterialLibraryCreateGroupCommand();
  }

  if(key == "ccxMaterialLibraryModifyGroupCommand")
  {
    return new ccxMaterialLibraryModifyGroupCommand();
  }
  
  if(key == "ccxMaterialLibraryDeleteGroupCommand")
  {
    return new ccxMaterialLibraryDeleteGroupCommand();
  }

  if(key == "ccxMaterialLibraryCreateMaterialCommand")
  {
    return new ccxMaterialLibraryCreateMaterialCommand();
  }

  if(key == "ccxMaterialLibraryModifyMaterialCommand")
  {
    return new ccxMaterialLibraryModifyMaterialCommand();
  }
  
  if(key == "ccxMaterialLibraryDeleteMaterialCommand")
  {
    return new ccxMaterialLibraryDeleteMaterialCommand();
  }

  if(key == "ccxMaterialLibraryExportMaterialCommand")
  {
    return new ccxMaterialLibraryExportMaterialCommand();
  }

  if(key == "ccxMaterialLibraryImportMaterialCommand")
  {
    return new ccxMaterialLibraryImportMaterialCommand();
  }
  
  if (key == "ccxWrapCommand")
  {
     return new ccxWrapCommand();
  }

  if (key == "ccxAnimateTopologyCommand")
  {
    return new ccxAnimateTopologyCommand();
  }

  return NULL;
}
