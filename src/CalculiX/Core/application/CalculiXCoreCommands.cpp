#include "Core/CalculiXCore.hpp"
#include "CoreApplicationLog.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <QScopedValueRollback>

bool CalculiXCore::cmd(std::string cmd)
{
  return CubitInterface::cmd_single(cmd.c_str());
}

bool CalculiXCore::silent_cmd(std::string cmd)
{
  const QScopedValueRollback<unsigned int> guard(silent_cmd_depth, silent_cmd_depth + 1);
  return CubitInterface::silent_cmd(cmd.c_str());
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
