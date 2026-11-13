#include "Core/CalculiXCore.hpp"
#include "CoreApplicationLog.hpp"
#include "Core/topology/CoreTopologyTrace.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <QScopedValueRollback>
#include <exception>

bool CalculiXCore::cmd(std::string cmd)
{
  return CubitInterface::cmd_single(cmd.c_str());
}

bool CalculiXCore::silent_cmd(std::string cmd)
{
  const QScopedValueRollback<unsigned int> guard(silent_cmd_depth, silent_cmd_depth + 1);
  CoreTopologyTrace::write("command.silent.enter");

  try
  {
    const bool result = CubitInterface::silent_cmd(cmd.c_str());
    CoreTopologyTrace::write(result ? "command.silent.success" : "command.silent.failure");
    return result;
  }
  catch (const std::exception& error)
  {
    CoreTopologyTrace::write("command.silent.exception");
    PRINT_ERROR("CalculiX silent command '%s' failed: %s\n", cmd.c_str(), error.what());
  }
  catch (...)
  {
    CoreTopologyTrace::write("command.silent.unexpected_exception");
    PRINT_ERROR("CalculiX silent command '%s' failed with an unexpected exception.\n", cmd.c_str());
  }

  return false;
}

bool CalculiXCore::print_to_log(std::string str_log)
{
  return CoreApplicationLog::append(str_log);
}

bool CalculiXCore::print_to_cl(std::string str_log)
{
  PRINT_INFO("%s", str_log.c_str());
  return true;
}
