#include "CalculiXPlugin.hpp"
#include "ccxVersionCommand.hpp"
#include "ccxExportCommand.hpp"
#include "ccxPrintCoreCommand.hpp"
#include "ccxLogCoreCommand.hpp"
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

//! This macro is required to identify this as a valid Cubit plugin. The plugin
//! will NOT be loaded if this macro is not present.
CUBIT_PLUGIN(CalculiXPlugin)

CalculiXPlugin::CalculiXPlugin()
{}

CalculiXPlugin::~CalculiXPlugin()
{}

std::vector<std::string> CalculiXPlugin::get_keys()
{
  std::vector<std::string> keys;
  keys.push_back("ccxVersionCommand");
  keys.push_back("ccxExportCommand");
  keys.push_back("ccxPrintCoreCommand");
  keys.push_back("ccxLogCoreCommand");
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

  return keys;
}

CubitCommand* CalculiXPlugin::create_command(const std::string &key)
{
  // NOTE: The internals of Cubit will take owernship of the command,
  // and delete it when it is time to clean up.

  if(key == "ccxVersionCommand")
    return new ccxVersionCommand();

  else if(key == "ccxExportCommand")
    return new ccxExportCommand();

  else if(key == "ccxPrintCoreCommand")
    return new ccxPrintCoreCommand();
  
  else if(key == "ccxLogCoreCommand")
    return new ccxLogCoreCommand();
  
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

  return NULL;
}
