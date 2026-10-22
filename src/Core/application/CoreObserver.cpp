#include "Core/application/CoreObserver.hpp"
#include <string>
#include "CalculiXApplicationInterface.hpp"
#include <QScopedValueRollback>

void CoreObserver::notify_command_start()
{
  if (!updating && command_start)
  {
    command_start();
  }
}

void CoreObserver::notify_command_complete()
{
  CalculiXApplicationInterface ccx_application_iface;

  if (updating)
  {
    return;
  }

  QScopedValueRollback<bool> guard(updating, true);

  if (before_core_update)
  {
    before_core_update();
  }

  if (ccx_application_iface.block_core_update())
  {
    return;
  }

  if (ccx_application_iface.core_update() && core_updated)
  {
    core_updated();
  }
}

void CoreObserver::notify_model_reset()
{
  CalculiXApplicationInterface ccx_application_iface;

  if (updating)
  {
    return;
  }

  QScopedValueRollback<bool> guard(updating, true);
  if (ccx_application_iface.core_reset() && core_reset)
  {
    core_reset();
  }
}

void CoreObserver::notify_cub_file_read(std::string filename)
{
  CalculiXApplicationInterface ccx_application_iface;
  ccx_application_iface.core_read_cub(filename);
}

void CoreObserver::notify_cub_file_saved(std::string filename, bool is_tmp_file)
{
  if (!is_tmp_file)
  {
    CalculiXApplicationInterface ccx_application_iface;
    ccx_application_iface.core_save_cub(filename);
  }
}
