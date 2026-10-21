#include "CoreObserver.hpp"
#include <QScopedValueRollback>

void CoreObserver::notify_command_start()
{
  if (!updating && command_start)
    command_start();
}

void CoreObserver::notify_command_complete()
{
  if (updating)
    return;

  QScopedValueRollback<bool> guard(updating, true);

  if (before_core_update)
    before_core_update();

  if (iface.block_core_update())
    return;

  if (iface.core_update() && core_updated)
    core_updated();
}

void CoreObserver::notify_model_reset()
{
  if (updating)
    return;

  QScopedValueRollback<bool> guard(updating, true);
  if (iface.core_reset() && core_reset)
    core_reset();
}

void CoreObserver::notify_cub_file_read(std::string filename)
{
  iface.core_read_cub(filename);
}

void CoreObserver::notify_cub_file_saved(std::string filename, bool is_tmp_file)
{
  if (!is_tmp_file)
    iface.core_save_cub(filename);
}
