#ifndef COREOBSERVER_HPP
#define COREOBSERVER_HPP
#include <string>

#include "CubitInterface.hpp"
#include <functional>

// Shared observer registered by the command plugin; GUI callbacks are optional.
class CoreObserver : public CubitInterface::ObserverBase
{
public:
  std::function<void()> command_start;
  std::function<void()> before_core_update;
  std::function<void()> core_updated;
  std::function<void()> core_reset;

  void notify_command_start() override;
  void notify_command_complete() override;
  void notify_model_reset() override;
  void notify_cub_file_read(std::string filename) override;
  void notify_cub_file_saved(std::string filename, bool is_tmp_file) override;

  bool is_updating() const { return updating; }

private:
  bool updating = false;
};

#endif
