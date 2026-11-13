#include "Core/application/CoreObserver.hpp"
#include <string>
#include "CalculiXApplicationInterface.hpp"
#include "Core/topology/CoreTopologyTrace.hpp"
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
  CoreTopologyTrace::write("observer.command_complete.enter");
  CalculiXApplicationInterface ccx_application_iface;

  if (updating)
  {
    CoreTopologyTrace::write("observer.command_complete.reentrant_skip");
    return;
  }

  if (ccx_application_iface.block_core_update())
  {
    CoreTopologyTrace::write("observer.command_complete.blocked_skip");
    return;
  }

  const QScopedValueRollback<bool> guard(updating, true);

  if (before_core_update)
  {
    CoreTopologyTrace::write("observer.before_core_update.enter");
    before_core_update();
    CoreTopologyTrace::write("observer.before_core_update.leave");
  }

  CoreTopologyTrace::write("observer.core_update.enter");
  const bool updated = ccx_application_iface.core_update();
  CoreTopologyTrace::write(updated ? "observer.core_update.success" : "observer.core_update.failure");

  if (updated && core_updated)
  {
    CoreTopologyTrace::write("observer.gui_callback.enter");
    core_updated();
    CoreTopologyTrace::write("observer.gui_callback.leave");
  }
  CoreTopologyTrace::write("observer.command_complete.leave");
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
  if (project_coordinated)
  {
    return;
  }
  CalculiXApplicationInterface ccx_application_iface;
  ccx_application_iface.core_read_cub(filename);
}

void CoreObserver::notify_cub_file_saved(std::string filename, bool is_tmp_file)
{
  if (!is_tmp_file && !project_coordinated)
  {
    CalculiXApplicationInterface ccx_application_iface;
    ccx_application_iface.core_save_cub(filename);
  }
}
