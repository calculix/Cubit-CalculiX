#include "CalculiXPlugin.hpp"
#include "ccxVersionCommand.hpp"
#include "ccxExportCommand.hpp"
#include "ccxPrintCoreCommand.hpp"
#include "ccxLogCoreCommand.hpp"
#include "ccxBlockElementTypeCommand.hpp"
#include "ccxSectionDeleteCommand.hpp"
#include "ccxSectionSolidCreateCommand.hpp"

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

  return NULL;
}
