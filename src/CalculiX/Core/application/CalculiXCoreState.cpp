#include "Core/CalculiXCore.hpp"

std::string CalculiXCore::get_version()
{
  return version;
}

bool CalculiXCore::is_silent_cmd() const
{
  return silent_cmd_depth != 0;
}

bool CalculiXCore::is_checking_jobs() const
{
  return checking_jobs;
}

bool CalculiXCore::gui_updated()
{
  return bool_gui_updated;
}

bool CalculiXCore::set_gui_updated(bool status)
{
  bool_gui_updated = status;
  return true;
}

bool CalculiXCore::block_core_update()
{
  return bool_block_core_update;
}

bool CalculiXCore::set_block_core_update(bool status)
{
  bool_block_core_update = status;
  return true;
}

bool CalculiXCore::block_gui_update()
{
  return bool_block_gui_update;
}

bool CalculiXCore::set_block_gui_update(bool status)
{
  bool_block_gui_update = status;
  return true;
}

bool CalculiXCore::init_completed()
{
  return bool_init && bool_init2 && bool_init_pythoninterface && bool_init_materiallibrary;
}
