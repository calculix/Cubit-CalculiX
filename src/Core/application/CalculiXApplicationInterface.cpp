#include "CalculiXApplicationInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"
#include "CoreObserver.hpp"

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
  ccx_core.print_to_log(str_log);
  return true;
}

bool CalculiXApplicationInterface::print_to_cl(std::string str_log)
{
  ccx_core.print_to_cl(str_log);
  return true;
}

bool CalculiXApplicationInterface::export_to_csv(std::string path, std::vector<std::string> header, std::vector<std::vector<double>> data,bool overwrite)
{
  return ccx_core.export_to_csv(path, header, data,overwrite);
}

std::vector<int> CalculiXApplicationInterface::parser(std::string parse_type, std::string parse_string)
{
  return ccx_core.parser(parse_type, parse_string);
}

std::string  CalculiXApplicationInterface::to_string_scientific(double value, int precision)
{
  return ccx_core.to_string_scientific(value, precision);
}

double CalculiXApplicationInterface::string_scientific_to_double(std::string value, int precision)
{
  return ccx_core.string_scientific_to_double(value, precision);
}

bool CalculiXApplicationInterface::core_update()
{
  if (ccx_core.update())
  {
    return true;
  }else{
    return false;
  }
}

bool CalculiXApplicationInterface::core_reset()
{
  if (ccx_core.reset())
  {
    return true;
  }else{
    return false;
  }
}

bool CalculiXApplicationInterface::core_read_cub(std::string filename)
{
  if (ccx_core.read_cub(filename))
  {
    return true;
  }else{
    return false;
  }
}

bool CalculiXApplicationInterface::core_save_cub(std::string filename)
{
  if (ccx_core.save_cub(filename))
  {
    return true;
  }else{
    return false;
  }
}
