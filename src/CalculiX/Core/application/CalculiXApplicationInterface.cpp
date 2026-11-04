#include "CalculiXApplicationInterface.hpp"
#include "CoreUserOptions.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"
#include "Core/export/CoreCsvWriter.hpp"
#include "Core/application/CoreApplicationLog.hpp"
#include "CoreObserver.hpp"
#include "ScientificNumber.hpp"

CoreObserver& CalculiXApplicationInterface::core_observer()
{
  static CoreObserver observer;
  return observer;
}

bool CalculiXApplicationInterface::cmd(std::string cmd)
{
  return ccx_core.cmd(cmd);
}

bool CalculiXApplicationInterface::silent_cmd(std::string cmd)
{
  return ccx_core.silent_cmd(cmd);
}

bool CalculiXApplicationInterface::is_silent_cmd() const
{
  return ccx_core.is_silent_cmd();
}

std::string CalculiXApplicationInterface::get_version()
{
  return ccx_core.get_version();
}

bool CalculiXApplicationInterface::set_use_ccx_autocleanup(bool bool_use)
{
  ccx_core.use_ccx_autocleanup = bool_use;
  return true;
}

bool CalculiXApplicationInterface::set_use_ccx_logfile(bool bool_use)
{
  ccx_core.use_ccx_logfile = bool_use;
  return true;
}

bool CalculiXApplicationInterface::init_pythoninterface()
{
  return ccx_core.init_pythoninterface();
}

bool CalculiXApplicationInterface::init_materiallibrary()
{
  return ccx_core.init_materiallibrary();
}

bool CalculiXApplicationInterface::init_completed()
{
  return ccx_core.init_completed();
}

bool CalculiXApplicationInterface::gui_updated()
{
  return ccx_core.gui_updated();
}

bool CalculiXApplicationInterface::set_gui_updated(bool status)
{
  return ccx_core.set_gui_updated(status);
}

bool CalculiXApplicationInterface::block_core_update()
{
  return ccx_core.block_core_update();
}

bool CalculiXApplicationInterface::set_block_core_update(bool status)
{
  return ccx_core.set_block_core_update(status);
}

bool CalculiXApplicationInterface::block_gui_update()
{
  return ccx_core.block_gui_update();
}

bool CalculiXApplicationInterface::set_block_gui_update(bool status)
{
  return ccx_core.set_block_gui_update(status);
}

std::string CalculiXApplicationInterface::print_data()
{
  return ccx_core.print_data();
}

bool CalculiXApplicationInterface::print_to_log(std::string str_log)
{
  return CoreApplicationLog::append(str_log);
}

bool CalculiXApplicationInterface::print_to_cl(std::string str_log)
{
  ccx_core.print_to_cl(str_log);
  return true;
}

bool CalculiXApplicationInterface::export_to_csv(std::string path, std::vector<std::string> header, std::vector<std::vector<double>> data,bool overwrite)
{
  return CoreCsvWriter::write(path, header, data, overwrite);
}

std::vector<int> CalculiXApplicationInterface::parser(std::string parse_type, std::string parse_string)
{
  return ccx_core.parser(parse_type, parse_string);
}

std::string CalculiXApplicationInterface::to_string_scientific(double value, int precision)
{
  return ScientificNumber::format(value, precision);
}

double CalculiXApplicationInterface::string_scientific_to_double(std::string value, int precision)
{
  return ScientificNumber::parse_with_precision(value, precision);
}

bool CalculiXApplicationInterface::core_update()
{
  return ccx_core.update();
}

bool CalculiXApplicationInterface::core_reset()
{
  return ccx_core.reset();
}

bool CalculiXApplicationInterface::core_read_cub(std::string filename)
{
  return ccx_core.read_cub(filename);
}

bool CalculiXApplicationInterface::core_save_cub(std::string filename)
{
  return ccx_core.save_cub(filename);
}

bool CalculiXApplicationInterface::initialize_application()
{
  if (!ccx_core.bool_init && !ccx_core.init())
  {
    return false;
  }

  return ccx_core.bool_init2 || ccx_core.init2();
}

bool CalculiXApplicationInterface::initialize_user_options(UserOptionsInterface& options, std::string& error)
{
  return CoreUserOptions::initialize(options, error);
}

void CalculiXApplicationInterface::shutdown_user_options() noexcept
{
  CoreUserOptions::detach();
}

UserOptions CalculiXApplicationInterface::user_options() const
{
  return CoreUserOptions::current();
}

std::vector<std::string> CalculiXApplicationInterface::user_options_restart_required() const
{
  return CoreUserOptions::restart_required();
}

std::string CalculiXApplicationInterface::component_directory() const
{
  return CoreUserOptions::component_directory();
}

bool CalculiXApplicationInterface::apply_user_options(const std::string& payload, std::string& error)
{
  return CoreUserOptions::apply(payload, error);
}
