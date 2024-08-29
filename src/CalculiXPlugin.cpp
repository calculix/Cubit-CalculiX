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
#include "ccxLoadsForcesModifyCommand.hpp"
#include "ccxLoadsPressuresModifyCommand.hpp"
#include "ccxLoadsHeatfluxesModifyCommand.hpp"
#include "ccxLoadsGravityCreateCommand.hpp"
#include "ccxLoadsGravityModifyCommand.hpp"
#include "ccxLoadsGravityDeleteCommand.hpp"
#include "ccxLoadsCentrifugalCreateCommand.hpp"
#include "ccxLoadsCentrifugalModifyCommand.hpp"
#include "ccxLoadsCentrifugalDeleteCommand.hpp"
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
#include "ccxResultProjectJobCommand.hpp"
#include "ccxKillJobCommand.hpp"
#include "ccxCustomLineCreateCommand.hpp"
#include "ccxCustomLineModifyCommand.hpp"
#include "ccxCustomLineDeleteCommand.hpp"

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
  keys.push_back("ccxLoadsForcesModifyCommand");
  keys.push_back("ccxLoadsPressuresModifyCommand");
  keys.push_back("ccxLoadsHeatfluxesModifyCommand");
  keys.push_back("ccxLoadsGravityCreateCommand");
  keys.push_back("ccxLoadsGravityModifyCommand");
  keys.push_back("ccxLoadsGravityDeleteCommand");
  keys.push_back("ccxLoadsCentrifugalCreateCommand");
  keys.push_back("ccxLoadsCentrifugalModifyCommand");
  keys.push_back("ccxLoadsCentrifugalDeleteCommand");
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
  keys.push_back("ccxResultProjectJobCommand");
  keys.push_back("ccxCustomLineCreateCommand");
  keys.push_back("ccxCustomLineModifyCommand");
  keys.push_back("ccxCustomLineDeleteCommand");

  return keys;
}

CubitCommand* CalculiXPlugin::create_command(const std::string &key)
{
  // NOTE: The internals of Cubit will take owernship of the command,
  // and delete it when it is time to clean up.

  if(key == "ccxVersionCommand")
    return new ccxVersionCommand();
  
  else if(key == "ccxResetCommand")
    return new ccxResetCommand();

  else if(key == "ccxExportCommand")
    return new ccxExportCommand();

  else if(key == "ccxUseAutoCleanupCommand")
    return new ccxUseAutoCleanupCommand();
  
  else if(key == "ccxUseLogCoreCommand")
    return new ccxUseLogCoreCommand();

  else if(key == "ccxPrintCoreCommand")
    return new ccxPrintCoreCommand();
  
  else if(key == "ccxLogCoreCommand")
    return new ccxLogCoreCommand();
  
  else if(key == "ccxDrawCommand")
    return new ccxDrawCommand();
  
  else if(key == "ccxDrawStepCommand")
    return new ccxDrawStepCommand();

  else if(key == "ccxBlockElementTypeCommand")
    return new ccxBlockElementTypeCommand();

  else if(key == "ccxSectionDeleteCommand")
    return new ccxSectionDeleteCommand();

  else if(key == "ccxSectionSolidCreateCommand")
    return new ccxSectionSolidCreateCommand();

  else if(key == "ccxSectionSolidModifyCommand")
    return new ccxSectionSolidModifyCommand();

  else if(key == "ccxSectionShellCreateCommand")
    return new ccxSectionShellCreateCommand();

  else if(key == "ccxSectionShellModifyCommand")
    return new ccxSectionShellModifyCommand();

  else if(key == "ccxSectionBeamCreateCommand")
    return new ccxSectionBeamCreateCommand();

  else if(key == "ccxSectionBeamModifyCommand")
    return new ccxSectionBeamModifyCommand();

  else if(key == "ccxSectionMembraneCreateCommand")
    return new ccxSectionMembraneCreateCommand();

  else if(key == "ccxSectionMembraneModifyCommand")
    return new ccxSectionMembraneModifyCommand();

  else if(key == "ccxConstraintDeleteCommand")
    return new ccxConstraintDeleteCommand();

  else if(key == "ccxConstraintRigidBodyCreateCommand")
    return new ccxConstraintRigidBodyCreateCommand();

  else if(key == "ccxConstraintRigidBody2CreateCommand")
    return new ccxConstraintRigidBody2CreateCommand();
  
  else if(key == "ccxConstraintRigidBodyModifyCommand")
    return new ccxConstraintRigidBodyModifyCommand();

  else if(key == "ccxConstraintTieCreateCommand")
    return new ccxConstraintTieCreateCommand();

  else if(key == "ccxConstraintTieModifyCommand")
    return new ccxConstraintTieModifyCommand();

  else if(key == "ccxConstraintTieCreateFromCubitContactPairCommand")
    return new ccxConstraintTieCreateFromCubitContactPairCommand();

  else if(key == "ccxSurfaceInteractionExponentialCreateCommand")
    return new ccxSurfaceInteractionExponentialCreateCommand();

  else if(key == "ccxSurfaceInteractionLinearCreateCommand")
    return new ccxSurfaceInteractionLinearCreateCommand();
  
  else if(key == "ccxSurfaceInteractionTabularCreateCommand")
    return new ccxSurfaceInteractionTabularCreateCommand();

  else if(key == "ccxSurfaceInteractionTiedCreateCommand")
    return new ccxSurfaceInteractionTiedCreateCommand();

  else if(key == "ccxSurfaceInteractionHardCreateCommand")
    return new ccxSurfaceInteractionHardCreateCommand();

  else if(key == "ccxSurfaceInteractionNameModifyCommand")
    return new ccxSurfaceInteractionNameModifyCommand();

  else if(key == "ccxSurfaceInteractionExponentialModifyCommand")
    return new ccxSurfaceInteractionExponentialModifyCommand();

  else if(key == "ccxSurfaceInteractionLinearModifyCommand")
    return new ccxSurfaceInteractionLinearModifyCommand();

  else if(key == "ccxSurfaceInteractionTabularModifyCommand")
    return new ccxSurfaceInteractionTabularModifyCommand();

  else if(key == "ccxSurfaceInteractionTiedModifyCommand")
    return new ccxSurfaceInteractionTiedModifyCommand();

  else if(key == "ccxSurfaceInteractionGapConductanceModifyCommand")
    return new ccxSurfaceInteractionGapConductanceModifyCommand();

  else if(key == "ccxSurfaceInteractionGapHeatGenerationModifyCommand")
    return new ccxSurfaceInteractionGapHeatGenerationModifyCommand();

  else if(key == "ccxSurfaceInteractionFrictionModifyCommand")
    return new ccxSurfaceInteractionFrictionModifyCommand();

  else if(key == "ccxSurfaceInteractionDeleteCommand")
    return new ccxSurfaceInteractionDeleteCommand();

  else if(key == "ccxContactPairCreateCommand")
    return new ccxContactPairCreateCommand();

  else if(key == "ccxContactPairModifyCommand")
    return new ccxContactPairModifyCommand();
  
  else if(key == "ccxContactPairDeleteCommand")
    return new ccxContactPairDeleteCommand();
  
  else if(key == "ccxContactPairCreateFromCubitContactPairCommand")
    return new ccxContactPairCreateFromCubitContactPairCommand();

  else if(key == "ccxAmplitudeCreateCommand")
    return new ccxAmplitudeCreateCommand();

  else if(key == "ccxAmplitudeModifyCommand")
    return new ccxAmplitudeModifyCommand();

  else if(key == "ccxAmplitudeDeleteCommand")
    return new ccxAmplitudeDeleteCommand();
  
  else if(key == "ccxOrientationCreateCommand")
    return new ccxOrientationCreateCommand();

  else if(key == "ccxOrientationModifyCommand")
    return new ccxOrientationModifyCommand();

  else if(key == "ccxOrientationDeleteCommand")
    return new ccxOrientationDeleteCommand();

  else if(key == "ccxLoadsForcesModifyCommand")
    return new ccxLoadsForcesModifyCommand();

  else if(key == "ccxLoadsPressuresModifyCommand")
    return new ccxLoadsPressuresModifyCommand();

  else if(key == "ccxLoadsHeatfluxesModifyCommand")
    return new ccxLoadsHeatfluxesModifyCommand();

  else if(key == "ccxLoadsGravityCreateCommand")
    return new ccxLoadsGravityCreateCommand();
  
  else if(key == "ccxLoadsGravityModifyCommand")
    return new ccxLoadsGravityModifyCommand();

  else if(key == "ccxLoadsGravityDeleteCommand")
    return new ccxLoadsGravityDeleteCommand();

  else if(key == "ccxLoadsCentrifugalCreateCommand")
    return new ccxLoadsCentrifugalCreateCommand();
  
  else if(key == "ccxLoadsCentrifugalModifyCommand")
    return new ccxLoadsCentrifugalModifyCommand();

  else if(key == "ccxLoadsCentrifugalDeleteCommand")
    return new ccxLoadsCentrifugalDeleteCommand();

  else if(key == "ccxBCsDisplacementsModifyCommand")
    return new ccxBCsDisplacementsModifyCommand();

  else if(key == "ccxBCsTemperaturesModifyCommand")
    return new ccxBCsTemperaturesModifyCommand();
  
  else if(key == "ccxHistoryOutputCreateCommand")
    return new ccxHistoryOutputCreateCommand();

  else if(key == "ccxHistoryOutputNodeModifyCommand")
    return new ccxHistoryOutputNodeModifyCommand();
  
  else if(key == "ccxHistoryOutputElementModifyCommand")
    return new ccxHistoryOutputElementModifyCommand();
  
  else if(key == "ccxHistoryOutputContactModifyCommand")
    return new ccxHistoryOutputContactModifyCommand();

  else if(key == "ccxHistoryOutputDeleteCommand")
    return new ccxHistoryOutputDeleteCommand();

  else if(key == "ccxFieldOutputCreateCommand")
    return new ccxFieldOutputCreateCommand();

  else if(key == "ccxFieldOutputNodeModifyCommand")
    return new ccxFieldOutputNodeModifyCommand();
  
  else if(key == "ccxFieldOutputElementModifyCommand")
    return new ccxFieldOutputElementModifyCommand();

  else if(key == "ccxFieldOutputContactModifyCommand")
    return new ccxFieldOutputContactModifyCommand();

  else if(key == "ccxFieldOutputDeleteCommand")
    return new ccxFieldOutputDeleteCommand();

  else if(key == "ccxInitialConditionCreateCommand")
    return new ccxInitialConditionCreateCommand();

  else if(key == "ccxInitialConditionDisplacementModifyCommand")
    return new ccxInitialConditionDisplacementModifyCommand();

  else if(key == "ccxInitialConditionTemperatureModifyCommand")
    return new ccxInitialConditionTemperatureModifyCommand();

  else if(key == "ccxInitialConditionDeleteCommand")
    return new ccxInitialConditionDeleteCommand();

  else if(key == "ccxHBCAddBCCommand")
    return new ccxHBCAddBCCommand();

  else if(key == "ccxHBCRemoveBCCommand")
    return new ccxHBCRemoveBCCommand();

  else if(key == "ccxStepCreateCommand")
    return new ccxStepCreateCommand();
  
  else if(key == "ccxStepAddLoadCommand")
    return new ccxStepAddLoadCommand();

  else if(key == "ccxStepAddBCCommand")
    return new ccxStepAddBCCommand();

  else if(key == "ccxStepAddHistoryOutputCommand")
    return new ccxStepAddHistoryOutputCommand();
  
  else if(key == "ccxStepAddFieldOutputCommand")
    return new ccxStepAddFieldOutputCommand();

  else if(key == "ccxStepRemoveLoadCommand")
    return new ccxStepRemoveLoadCommand();

  else if(key == "ccxStepRemoveBCCommand")
    return new ccxStepRemoveBCCommand();

  else if(key == "ccxStepRemoveHistoryOutputCommand")
    return new ccxStepRemoveHistoryOutputCommand();
  
  else if(key == "ccxStepRemoveFieldOutputCommand")
    return new ccxStepRemoveFieldOutputCommand();
  
  else if(key == "ccxStepParameterModifyCommand")
    return new ccxStepParameterModifyCommand();

  else if(key == "ccxStepStaticModifyCommand")
    return new ccxStepStaticModifyCommand();

  else if(key == "ccxStepFrequencyModifyCommand")
    return new ccxStepFrequencyModifyCommand();

  else if(key == "ccxStepBuckleModifyCommand")
    return new ccxStepBuckleModifyCommand();

  else if(key == "ccxStepHeatTransferModifyCommand")
    return new ccxStepHeatTransferModifyCommand();

  else if(key == "ccxStepCoupledTemperatureDisplacementModifyCommand")
    return new ccxStepCoupledTemperatureDisplacementModifyCommand();  

  else if(key == "ccxStepUncoupledTemperatureDisplacementModifyCommand")
    return new ccxStepUncoupledTemperatureDisplacementModifyCommand();  

  else if(key == "ccxStepDeleteCommand")
    return new ccxStepDeleteCommand();
  
  else if(key == "ccxJobCreateCommand")
    return new ccxJobCreateCommand();

  else if(key == "ccxJobModifyCommand")
    return new ccxJobModifyCommand();
  
  else if(key == "ccxJobDeleteCommand")
    return new ccxJobDeleteCommand();

  else if(key == "ccxRunJobCommand")
    return new ccxRunJobCommand();
  
  else if(key == "ccxWaitJobCommand")
    return new ccxWaitJobCommand();
  
  else if(key == "ccxResultCGXJobCommand")
    return new ccxResultCGXJobCommand();

  else if(key == "ccxResultParaViewJobCommand")
    return new ccxResultParaViewJobCommand();

  else if(key == "ccxResultLoadJobCommand")
    return new ccxResultLoadJobCommand();

  else if(key == "ccxResultConvertJobCommand")
    return new ccxResultConvertJobCommand();

  else if(key == "ccxResultProjectJobCommand")
    return new ccxResultProjectJobCommand();

  else if(key == "ccxResultPlotJobFrdCommand")
    return new ccxResultPlotJobFrdCommand();

  else if(key == "ccxResultPlotJobDatCommand")
    return new ccxResultPlotJobDatCommand();

  else if(key == "ccxKillJobCommand")
    return new ccxKillJobCommand();

  else if(key == "ccxCustomLineCreateCommand")
    return new ccxCustomLineCreateCommand();

  else if(key == "ccxCustomLineModifyCommand")
    return new ccxCustomLineModifyCommand();
  
  else if(key == "ccxCustomLineDeleteCommand")
    return new ccxCustomLineDeleteCommand();

  return NULL;
}