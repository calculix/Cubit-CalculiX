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

  return NULL;
}
