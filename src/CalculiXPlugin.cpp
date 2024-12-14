#include "CalculiXPlugin.hpp"
#include "loadCalculiXCore.hpp"

#include "ccxVersionCommand.hpp"
#include "ccxResetCommand.hpp"
#include "ccxExportCommand.hpp"
#include "ccxUseAutoCleanupCommand.hpp"
#include "ccxUseLogCoreCommand.hpp"
#include "ccxPrintCoreCommand.hpp"
#include "ccxLogCoreCommand.hpp"
#include "ccxDrawCommand.hpp"
#include "ccxDrawStepCommand.hpp"
#include "ccxBlockElementTypeCommand.hpp"
#include "ccxSectionDeleteCommand.hpp"
#include "ccxSectionSolidCreateCommand.hpp"
#include "ccxSectionSolidModifyCommand.hpp"
#include "ccxSectionShellCreateCommand.hpp"
#include "ccxSectionShellModifyCommand.hpp"
#include "ccxSectionBeamCreateCommand.hpp"
#include "ccxSectionBeamModifyCommand.hpp"
#include "ccxSectionMembraneCreateCommand.hpp"
#include "ccxSectionMembraneModifyCommand.hpp"
#include "ccxConstraintDeleteCommand.hpp"
#include "ccxConstraintRigidBodyCreateCommand.hpp"
#include "ccxConstraintRigidBody2CreateCommand.hpp"
#include "ccxConstraintRigidBodyModifyCommand.hpp"
#include "ccxConstraintTieCreateCommand.hpp"
#include "ccxConstraintTieModifyCommand.hpp"
#include "ccxConstraintTieCreateFromCubitContactPairCommand.hpp"
#include "ccxSurfaceInteractionExponentialCreateCommand.hpp"
#include "ccxSurfaceInteractionLinearCreateCommand.hpp"
#include "ccxSurfaceInteractionTabularCreateCommand.hpp"
#include "ccxSurfaceInteractionTiedCreateCommand.hpp"
#include "ccxSurfaceInteractionHardCreateCommand.hpp"
#include "ccxSurfaceInteractionNameModifyCommand.hpp"
#include "ccxSurfaceInteractionExponentialModifyCommand.hpp"
#include "ccxSurfaceInteractionLinearModifyCommand.hpp"
#include "ccxSurfaceInteractionTabularModifyCommand.hpp"
#include "ccxSurfaceInteractionTiedModifyCommand.hpp"
#include "ccxSurfaceInteractionGapConductanceModifyCommand.hpp"
#include "ccxSurfaceInteractionGapHeatGenerationModifyCommand.hpp"
#include "ccxSurfaceInteractionFrictionModifyCommand.hpp"
#include "ccxSurfaceInteractionDeleteCommand.hpp"
#include "ccxContactPairCreateCommand.hpp"
#include "ccxContactPairModifyCommand.hpp"
#include "ccxContactPairDeleteCommand.hpp"
#include "ccxContactPairCreateFromCubitContactPairCommand.hpp"
#include "ccxAmplitudeCreateCommand.hpp"
#include "ccxAmplitudeModifyCommand.hpp"
#include "ccxAmplitudeDeleteCommand.hpp"
#include "ccxOrientationCreateCommand.hpp"
#include "ccxOrientationModifyCommand.hpp"
#include "ccxOrientationDeleteCommand.hpp"
#include "ccxDampingModifyCommand.hpp"
#include "ccxDampingDeleteCommand.hpp"
#include "ccxPhysicalConstantsModifyCommand.hpp"
#include "ccxPhysicalConstantsDeleteCommand.hpp"
#include "ccxLoadsForcesModifyCommand.hpp"
#include "ccxLoadsPressuresModifyCommand.hpp"
#include "ccxLoadsHeatfluxesModifyCommand.hpp"
#include "ccxLoadsGravityCreateCommand.hpp"
#include "ccxLoadsGravityModifyCommand.hpp"
#include "ccxLoadsGravityDeleteCommand.hpp"
#include "ccxLoadsCentrifugalCreateCommand.hpp"
#include "ccxLoadsCentrifugalModifyCommand.hpp"
#include "ccxLoadsCentrifugalDeleteCommand.hpp"
#include "ccxLoadsTrajectoryCreateCommand.hpp"
#include "ccxLoadsTrajectoryModifyCommand.hpp"
#include "ccxLoadsTrajectoryDeleteCommand.hpp"
#include "ccxLoadsFilmCreateCommand.hpp"
#include "ccxLoadsFilmModifyCommand.hpp"
#include "ccxLoadsFilmDeleteCommand.hpp"
#include "ccxLoadsRadiationCreateCommand.hpp"
#include "ccxLoadsRadiationModifyCommand.hpp"
#include "ccxLoadsRadiationDeleteCommand.hpp"
#include "ccxBCsDisplacementsModifyCommand.hpp"
#include "ccxBCsTemperaturesModifyCommand.hpp"
#include "ccxHistoryOutputCreateCommand.hpp"
#include "ccxHistoryOutputNodeModifyCommand.hpp"
#include "ccxHistoryOutputElementModifyCommand.hpp"
#include "ccxHistoryOutputContactModifyCommand.hpp"
#include "ccxHistoryOutputDeleteCommand.hpp"
#include "ccxFieldOutputCreateCommand.hpp"
#include "ccxFieldOutputNodeModifyCommand.hpp"
#include "ccxFieldOutputElementModifyCommand.hpp"
#include "ccxFieldOutputContactModifyCommand.hpp"
#include "ccxFieldOutputDeleteCommand.hpp"
#include "ccxInitialConditionCreateCommand.hpp"
#include "ccxInitialConditionDisplacementModifyCommand.hpp"
#include "ccxInitialConditionTemperatureModifyCommand.hpp"
#include "ccxInitialConditionDeleteCommand.hpp"
#include "ccxHBCAddBCCommand.hpp"
#include "ccxHBCRemoveBCCommand.hpp"
#include "ccxStepCreateCommand.hpp"
#include "ccxStepAddLoadCommand.hpp"
#include "ccxStepAddBCCommand.hpp"
#include "ccxStepAddHistoryOutputCommand.hpp"
#include "ccxStepAddFieldOutputCommand.hpp"
#include "ccxStepRemoveLoadCommand.hpp"
#include "ccxStepRemoveBCCommand.hpp"
#include "ccxStepRemoveHistoryOutputCommand.hpp"
#include "ccxStepRemoveFieldOutputCommand.hpp"
#include "ccxStepParameterModifyCommand.hpp"
#include "ccxStepStaticModifyCommand.hpp"
#include "ccxStepFrequencyModifyCommand.hpp"
#include "ccxStepBuckleModifyCommand.hpp"
#include "ccxStepHeatTransferModifyCommand.hpp"
#include "ccxStepCoupledTemperatureDisplacementModifyCommand.hpp"
#include "ccxStepUncoupledTemperatureDisplacementModifyCommand.hpp"
#include "ccxStepDeleteCommand.hpp"
#include "ccxJobCreateCommand.hpp"
#include "ccxJobModifyCommand.hpp"
#include "ccxJobDeleteCommand.hpp"
#include "ccxRunJobCommand.hpp"
#include "ccxWaitJobCommand.hpp"
#include "ccxResultCGXJobCommand.hpp"
#include "ccxResultParaViewJobCommand.hpp"
#include "ccxResultLoadJobCommand.hpp"
#include "ccxResultConvertJobCommand.hpp"
#include "ccxResultPlotJobFrdCommand.hpp"
#include "ccxResultPlotJobDatCommand.hpp"
#include "ccxResultCSVJobFrdCommand.hpp"
#include "ccxResultCSVJobDatCommand.hpp"
#include "ccxResultProjectJobCommand.hpp"
#include "ccxKillJobCommand.hpp"
#include "ccxCustomLineCreateCommand.hpp"
#include "ccxCustomLineModifyCommand.hpp"
#include "ccxCustomLineDeleteCommand.hpp"
#include "ccxMaterialLibraryCreateGroupCommand.hpp"
#include "ccxMaterialLibraryModifyGroupCommand.hpp"
#include "ccxMaterialLibraryDeleteGroupCommand.hpp"
#include "ccxMaterialLibraryCreateMaterialCommand.hpp"
#include "ccxMaterialLibraryModifyMaterialCommand.hpp"
#include "ccxMaterialLibraryDeleteMaterialCommand.hpp"
#include "ccxMaterialLibraryExportMaterialCommand.hpp"
#include "ccxMaterialLibraryImportMaterialCommand.hpp"


//! This macro is required to identify this as a valid Cubit plugin. The plugin
//! will NOT be loaded if this macro is not present.
CUBIT_PLUGIN(CalculiXPlugin)

CalculiXPlugin::CalculiXPlugin()
{
  ccx_core.init2(); // to init missing material core and interfaces
}

CalculiXPlugin::~CalculiXPlugin()
{}

std::vector<std::string> CalculiXPlugin::get_keys()
{
  std::vector<std::string> keys;
  keys.push_back("ccxVersionCommand");
  keys.push_back("ccxResetCommand");
  keys.push_back("ccxExportCommand");
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
  keys.push_back("ccxLoadsTrajectoryCreateCommand");
  keys.push_back("ccxLoadsTrajectoryModifyCommand");
  keys.push_back("ccxLoadsTrajectoryDeleteCommand");
  keys.push_back("ccxLoadsFilmCreateCommand");
  keys.push_back("ccxLoadsFilmModifyCommand");
  keys.push_back("ccxLoadsFilmDeleteCommand");
  keys.push_back("ccxLoadsRadiationCreateCommand");
  keys.push_back("ccxLoadsRadiationModifyCommand");
  keys.push_back("ccxLoadsRadiationDeleteCommand");
  keys.push_back("ccxBCsDisplacementsModifyCommand");
  keys.push_back("ccxBCsTemperaturesModifyCommand");
  keys.push_back("ccxHistoryOutputCreateCommand");
  keys.push_back("ccxHistoryOutputNodeModifyCommand");
  keys.push_back("ccxHistoryOutputElementModifyCommand");
  keys.push_back("ccxHistoryOutputContactModifyCommand");
  keys.push_back("ccxHistoryOutputDeleteCommand");
  keys.push_back("ccxFieldOutputCreateCommand");
  keys.push_back("ccxFieldOutputNodeModifyCommand");
  keys.push_back("ccxFieldOutputElementModifyCommand");
  keys.push_back("ccxFieldOutputContactModifyCommand");
  keys.push_back("ccxFieldOutputDeleteCommand");
  keys.push_back("ccxInitialConditionCreateCommand");
  keys.push_back("ccxInitialConditionDisplacementModifyCommand");
  keys.push_back("ccxInitialConditionTemperatureModifyCommand");
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
  keys.push_back("ccxStepDeleteCommand");
  keys.push_back("ccxJobCreateCommand");
  keys.push_back("ccxJobModifyCommand");
  keys.push_back("ccxJobDeleteCommand");
  keys.push_back("ccxRunJobCommand");
  keys.push_back("ccxWaitJobCommand");
  keys.push_back("ccxKillJobCommand");
  keys.push_back("ccxResultCGXJobCommand");
  keys.push_back("ccxResultParaViewJobCommand");
  keys.push_back("ccxResultLoadJobCommand");
  keys.push_back("ccxResultConvertJobCommand");
  keys.push_back("ccxResultPlotJobFrdCommand");
  keys.push_back("ccxResultPlotJobDatCommand");
  keys.push_back("ccxResultCSVJobFrdCommand");
  keys.push_back("ccxResultCSVJobDatCommand");
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

  return keys;
}

CubitCommand* CalculiXPlugin::create_command(const std::string &key)
{
  // NOTE: The internals of Cubit will take owernship of the command,
  // and delete it when it is time to clean up.

  if(key == "ccxVersionCommand")
    return new ccxVersionCommand();
  
  if(key == "ccxResetCommand")
    return new ccxResetCommand();

  if(key == "ccxExportCommand")
    return new ccxExportCommand();

  if(key == "ccxUseAutoCleanupCommand")
    return new ccxUseAutoCleanupCommand();
  
  if(key == "ccxUseLogCoreCommand")
    return new ccxUseLogCoreCommand();

  if(key == "ccxPrintCoreCommand")
    return new ccxPrintCoreCommand();
  
  if(key == "ccxLogCoreCommand")
    return new ccxLogCoreCommand();
  
  if(key == "ccxDrawCommand")
    return new ccxDrawCommand();
  
  if(key == "ccxDrawStepCommand")
    return new ccxDrawStepCommand();

  if(key == "ccxBlockElementTypeCommand")
    return new ccxBlockElementTypeCommand();

  if(key == "ccxSectionDeleteCommand")
    return new ccxSectionDeleteCommand();

  if(key == "ccxSectionSolidCreateCommand")
    return new ccxSectionSolidCreateCommand();

  if(key == "ccxSectionSolidModifyCommand")
    return new ccxSectionSolidModifyCommand();

  if(key == "ccxSectionShellCreateCommand")
    return new ccxSectionShellCreateCommand();

  if(key == "ccxSectionShellModifyCommand")
    return new ccxSectionShellModifyCommand();

  if(key == "ccxSectionBeamCreateCommand")
    return new ccxSectionBeamCreateCommand();

  if(key == "ccxSectionBeamModifyCommand")
    return new ccxSectionBeamModifyCommand();

  if(key == "ccxSectionMembraneCreateCommand")
    return new ccxSectionMembraneCreateCommand();

  if(key == "ccxSectionMembraneModifyCommand")
    return new ccxSectionMembraneModifyCommand();

  if(key == "ccxConstraintDeleteCommand")
    return new ccxConstraintDeleteCommand();

  if(key == "ccxConstraintRigidBodyCreateCommand")
    return new ccxConstraintRigidBodyCreateCommand();

  if(key == "ccxConstraintRigidBody2CreateCommand")
    return new ccxConstraintRigidBody2CreateCommand();
  
  if(key == "ccxConstraintRigidBodyModifyCommand")
    return new ccxConstraintRigidBodyModifyCommand();

  if(key == "ccxConstraintTieCreateCommand")
    return new ccxConstraintTieCreateCommand();

  if(key == "ccxConstraintTieModifyCommand")
    return new ccxConstraintTieModifyCommand();

  if(key == "ccxConstraintTieCreateFromCubitContactPairCommand")
    return new ccxConstraintTieCreateFromCubitContactPairCommand();

  if(key == "ccxSurfaceInteractionExponentialCreateCommand")
    return new ccxSurfaceInteractionExponentialCreateCommand();

  if(key == "ccxSurfaceInteractionLinearCreateCommand")
    return new ccxSurfaceInteractionLinearCreateCommand();
  
  if(key == "ccxSurfaceInteractionTabularCreateCommand")
    return new ccxSurfaceInteractionTabularCreateCommand();

  if(key == "ccxSurfaceInteractionTiedCreateCommand")
    return new ccxSurfaceInteractionTiedCreateCommand();

  if(key == "ccxSurfaceInteractionHardCreateCommand")
    return new ccxSurfaceInteractionHardCreateCommand();

  if(key == "ccxSurfaceInteractionNameModifyCommand")
    return new ccxSurfaceInteractionNameModifyCommand();

  if(key == "ccxSurfaceInteractionExponentialModifyCommand")
    return new ccxSurfaceInteractionExponentialModifyCommand();

  if(key == "ccxSurfaceInteractionLinearModifyCommand")
    return new ccxSurfaceInteractionLinearModifyCommand();

  if(key == "ccxSurfaceInteractionTabularModifyCommand")
    return new ccxSurfaceInteractionTabularModifyCommand();

  if(key == "ccxSurfaceInteractionTiedModifyCommand")
    return new ccxSurfaceInteractionTiedModifyCommand();

  if(key == "ccxSurfaceInteractionGapConductanceModifyCommand")
    return new ccxSurfaceInteractionGapConductanceModifyCommand();

  if(key == "ccxSurfaceInteractionGapHeatGenerationModifyCommand")
    return new ccxSurfaceInteractionGapHeatGenerationModifyCommand();

  if(key == "ccxSurfaceInteractionFrictionModifyCommand")
    return new ccxSurfaceInteractionFrictionModifyCommand();

  if(key == "ccxSurfaceInteractionDeleteCommand")
    return new ccxSurfaceInteractionDeleteCommand();

  if(key == "ccxContactPairCreateCommand")
    return new ccxContactPairCreateCommand();

  if(key == "ccxContactPairModifyCommand")
    return new ccxContactPairModifyCommand();
  
  if(key == "ccxContactPairDeleteCommand")
    return new ccxContactPairDeleteCommand();
  
  if(key == "ccxContactPairCreateFromCubitContactPairCommand")
    return new ccxContactPairCreateFromCubitContactPairCommand();

  if(key == "ccxAmplitudeCreateCommand")
    return new ccxAmplitudeCreateCommand();

  if(key == "ccxAmplitudeModifyCommand")
    return new ccxAmplitudeModifyCommand();

  if(key == "ccxAmplitudeDeleteCommand")
    return new ccxAmplitudeDeleteCommand();
  
  if(key == "ccxOrientationCreateCommand")
    return new ccxOrientationCreateCommand();

  if(key == "ccxOrientationModifyCommand")
    return new ccxOrientationModifyCommand();

  if(key == "ccxOrientationDeleteCommand")
    return new ccxOrientationDeleteCommand();

  if(key == "ccxDampingModifyCommand")
    return new ccxDampingModifyCommand();

  if(key == "ccxDampingDeleteCommand")
    return new ccxDampingDeleteCommand();

  if(key == "ccxPhysicalConstantsModifyCommand")
    return new ccxPhysicalConstantsModifyCommand();

  if(key == "ccxPhysicalConstantsDeleteCommand")
    return new ccxPhysicalConstantsDeleteCommand();

  if(key == "ccxLoadsForcesModifyCommand")
    return new ccxLoadsForcesModifyCommand();

  if(key == "ccxLoadsPressuresModifyCommand")
    return new ccxLoadsPressuresModifyCommand();

  if(key == "ccxLoadsHeatfluxesModifyCommand")
    return new ccxLoadsHeatfluxesModifyCommand();

  if(key == "ccxLoadsGravityCreateCommand")
    return new ccxLoadsGravityCreateCommand();
  
  if(key == "ccxLoadsGravityModifyCommand")
    return new ccxLoadsGravityModifyCommand();

  if(key == "ccxLoadsGravityDeleteCommand")
    return new ccxLoadsGravityDeleteCommand();

  if(key == "ccxLoadsCentrifugalCreateCommand")
    return new ccxLoadsCentrifugalCreateCommand();
  
  if(key == "ccxLoadsCentrifugalModifyCommand")
    return new ccxLoadsCentrifugalModifyCommand();

  if(key == "ccxLoadsCentrifugalDeleteCommand")
    return new ccxLoadsCentrifugalDeleteCommand();

  if(key == "ccxLoadsTrajectoryCreateCommand")
    return new ccxLoadsTrajectoryCreateCommand();
  
  if(key == "ccxLoadsTrajectoryModifyCommand")
    return new ccxLoadsTrajectoryModifyCommand();

  if(key == "ccxLoadsTrajectoryDeleteCommand")
    return new ccxLoadsTrajectoryDeleteCommand();

  if(key == "ccxLoadsFilmCreateCommand")
    return new ccxLoadsFilmCreateCommand();

  if(key == "ccxLoadsFilmModifyCommand")
    return new ccxLoadsFilmModifyCommand();

  if(key == "ccxLoadsFilmDeleteCommand")
    return new ccxLoadsFilmDeleteCommand();

  if(key == "ccxLoadsRadiationCreateCommand")
    return new ccxLoadsRadiationCreateCommand();

  if(key == "ccxLoadsRadiationModifyCommand")
    return new ccxLoadsRadiationModifyCommand();

  if(key == "ccxLoadsRadiationDeleteCommand")
    return new ccxLoadsRadiationDeleteCommand();
    
  if(key == "ccxBCsDisplacementsModifyCommand")
    return new ccxBCsDisplacementsModifyCommand();

  if(key == "ccxBCsTemperaturesModifyCommand")
    return new ccxBCsTemperaturesModifyCommand();
  
  if(key == "ccxHistoryOutputCreateCommand")
    return new ccxHistoryOutputCreateCommand();

  if(key == "ccxHistoryOutputNodeModifyCommand")
    return new ccxHistoryOutputNodeModifyCommand();
  
  if(key == "ccxHistoryOutputElementModifyCommand")
    return new ccxHistoryOutputElementModifyCommand();
  
  if(key == "ccxHistoryOutputContactModifyCommand")
    return new ccxHistoryOutputContactModifyCommand();

  if(key == "ccxHistoryOutputDeleteCommand")
    return new ccxHistoryOutputDeleteCommand();

  if(key == "ccxFieldOutputCreateCommand")
    return new ccxFieldOutputCreateCommand();

  if(key == "ccxFieldOutputNodeModifyCommand")
    return new ccxFieldOutputNodeModifyCommand();
  
  if(key == "ccxFieldOutputElementModifyCommand")
    return new ccxFieldOutputElementModifyCommand();

  if(key == "ccxFieldOutputContactModifyCommand")
    return new ccxFieldOutputContactModifyCommand();

  if(key == "ccxFieldOutputDeleteCommand")
    return new ccxFieldOutputDeleteCommand();

  if(key == "ccxInitialConditionCreateCommand")
    return new ccxInitialConditionCreateCommand();

  if(key == "ccxInitialConditionDisplacementModifyCommand")
    return new ccxInitialConditionDisplacementModifyCommand();

  if(key == "ccxInitialConditionTemperatureModifyCommand")
    return new ccxInitialConditionTemperatureModifyCommand();

  if(key == "ccxInitialConditionDeleteCommand")
    return new ccxInitialConditionDeleteCommand();

  if(key == "ccxHBCAddBCCommand")
    return new ccxHBCAddBCCommand();

  if(key == "ccxHBCRemoveBCCommand")
    return new ccxHBCRemoveBCCommand();

  if(key == "ccxStepCreateCommand")
    return new ccxStepCreateCommand();
  
  if(key == "ccxStepAddLoadCommand")
    return new ccxStepAddLoadCommand();

  if(key == "ccxStepAddBCCommand")
    return new ccxStepAddBCCommand();

  if(key == "ccxStepAddHistoryOutputCommand")
    return new ccxStepAddHistoryOutputCommand();
  
  if(key == "ccxStepAddFieldOutputCommand")
    return new ccxStepAddFieldOutputCommand();

  if(key == "ccxStepRemoveLoadCommand")
    return new ccxStepRemoveLoadCommand();

  if(key == "ccxStepRemoveBCCommand")
    return new ccxStepRemoveBCCommand();

  if(key == "ccxStepRemoveHistoryOutputCommand")
    return new ccxStepRemoveHistoryOutputCommand();
  
  if(key == "ccxStepRemoveFieldOutputCommand")
    return new ccxStepRemoveFieldOutputCommand();
  
  if(key == "ccxStepParameterModifyCommand")
    return new ccxStepParameterModifyCommand();

  if(key == "ccxStepStaticModifyCommand")
    return new ccxStepStaticModifyCommand();

  if(key == "ccxStepFrequencyModifyCommand")
    return new ccxStepFrequencyModifyCommand();

  if(key == "ccxStepBuckleModifyCommand")
    return new ccxStepBuckleModifyCommand();

  if(key == "ccxStepHeatTransferModifyCommand")
    return new ccxStepHeatTransferModifyCommand();

  if(key == "ccxStepCoupledTemperatureDisplacementModifyCommand")
    return new ccxStepCoupledTemperatureDisplacementModifyCommand();  

  if(key == "ccxStepUncoupledTemperatureDisplacementModifyCommand")
    return new ccxStepUncoupledTemperatureDisplacementModifyCommand();  

  if(key == "ccxStepDeleteCommand")
    return new ccxStepDeleteCommand();
  
  if(key == "ccxJobCreateCommand")
    return new ccxJobCreateCommand();

  if(key == "ccxJobModifyCommand")
    return new ccxJobModifyCommand();
  
  if(key == "ccxJobDeleteCommand")
    return new ccxJobDeleteCommand();

  if(key == "ccxRunJobCommand")
    return new ccxRunJobCommand();
  
  if(key == "ccxWaitJobCommand")
    return new ccxWaitJobCommand();
  
  if(key == "ccxResultCGXJobCommand")
    return new ccxResultCGXJobCommand();

  if(key == "ccxResultParaViewJobCommand")
    return new ccxResultParaViewJobCommand();

  if(key == "ccxResultLoadJobCommand")
    return new ccxResultLoadJobCommand();

  if(key == "ccxResultConvertJobCommand")
    return new ccxResultConvertJobCommand();

  if(key == "ccxResultProjectJobCommand")
    return new ccxResultProjectJobCommand();

  if(key == "ccxResultPlotJobFrdCommand")
    return new ccxResultPlotJobFrdCommand();

  if(key == "ccxResultPlotJobDatCommand")
    return new ccxResultPlotJobDatCommand();

  if(key == "ccxResultCSVJobFrdCommand")
    return new ccxResultCSVJobFrdCommand();

  if(key == "ccxResultCSVJobDatCommand")
    return new ccxResultCSVJobDatCommand();

  if(key == "ccxKillJobCommand")
    return new ccxKillJobCommand();

  if(key == "ccxCustomLineCreateCommand")
    return new ccxCustomLineCreateCommand();

  if(key == "ccxCustomLineModifyCommand")
    return new ccxCustomLineModifyCommand();
  
  if(key == "ccxCustomLineDeleteCommand")
    return new ccxCustomLineDeleteCommand();

  if(key == "ccxMaterialLibraryCreateGroupCommand")
    return new ccxMaterialLibraryCreateGroupCommand();

  if(key == "ccxMaterialLibraryModifyGroupCommand")
    return new ccxMaterialLibraryModifyGroupCommand();
  
  if(key == "ccxMaterialLibraryDeleteGroupCommand")
    return new ccxMaterialLibraryDeleteGroupCommand();

  if(key == "ccxMaterialLibraryCreateMaterialCommand")
    return new ccxMaterialLibraryCreateMaterialCommand();

  if(key == "ccxMaterialLibraryModifyMaterialCommand")
    return new ccxMaterialLibraryModifyMaterialCommand();
  
  if(key == "ccxMaterialLibraryDeleteMaterialCommand")
    return new ccxMaterialLibraryDeleteMaterialCommand();

  if(key == "ccxMaterialLibraryExportMaterialCommand")
    return new ccxMaterialLibraryExportMaterialCommand();

  if(key == "ccxMaterialLibraryImportMaterialCommand")
    return new ccxMaterialLibraryImportMaterialCommand();

  return NULL;
}