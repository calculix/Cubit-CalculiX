#include "CoreTopologyProgress.hpp"
#include "CoreTopologyTrace.hpp"
#include "AppUtil.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"

#include <algorithm>

CoreTopologyProgress::CoreTopologyProgress(const char* title, std::size_t total)
  : total(total), previous(std::chrono::steady_clock::now())
{
  CoreTopologyTrace::write("progress.app_util.enter");
  auto* app = CubitInterface::app_util().get();
  CoreTopologyTrace::write("progress.app_util.leave");
  if (app)
  {
    CoreTopologyTrace::write("progress.progress_tool.enter");
    progress = app->progress_tool();
    CoreTopologyTrace::write("progress.progress_tool.leave");
  }

  if (progress)
  {
    CoreTopologyTrace::write(title);
    CoreTopologyTrace::write("progress.start.enter");
    progress->start(0, 100, title);
    CoreTopologyTrace::write("progress.start.leave");
    CoreTopologyTrace::write("progress.initial_percent.enter");
    progress->percent(0.0);
    CoreTopologyTrace::write("progress.initial_percent.leave");
  }
}

CoreTopologyProgress::~CoreTopologyProgress()
{
  if (progress)
  {
    CoreTopologyTrace::write("progress.end.enter");
    progress->end();
    CoreTopologyTrace::write("progress.end.leave");
  }
}

bool CoreTopologyProgress::poll()
{
  if (interrupted)
  {
    return false;
  }
  if (!progress)
  {
    return true;
  }

  const auto now = std::chrono::steady_clock::now();
  if (now - previous < std::chrono::milliseconds(200))
  {
    return true;
  }
  previous = now;

  const double fraction = total > 0
    ? static_cast<double>(completed) / static_cast<double>(total)
    : 0.0;
  CoreTopologyTrace::write("progress.periodic_percent.enter");
  progress->percent((std::min)(0.99, fraction));
  CoreTopologyTrace::write("progress.periodic_percent.leave");
  return check_interrupt();
}

bool CoreTopologyProgress::advance()
{
  if (completed < total)
  {
    ++completed;
  }
  return poll();
}

bool CoreTopologyProgress::finish()
{
  if (interrupted)
  {
    return false;
  }
  if (completed != total)
  {
    PRINT_ERROR("Topology progress finished before all elements were processed.\n");
    return false;
  }
  if (progress)
  {
    if (!check_interrupt())
    {
      return false;
    }
    CoreTopologyTrace::write("progress.final_percent.enter");
    progress->percent(1.0);
    CoreTopologyTrace::write("progress.final_percent.leave");
  }
  return true;
}

bool CoreTopologyProgress::check_interrupt()
{
  CoreTopologyTrace::write("progress.check_interrupt.enter");
  progress->check_interrupt();
  CoreTopologyTrace::write("progress.check_interrupt.leave");
  CoreTopologyTrace::write("progress.interrupt_app_util.enter");
  auto* app = CubitInterface::app_util().get();
  CoreTopologyTrace::write("progress.interrupt_app_util.leave");
  CoreTopologyTrace::write("progress.interrupt_flag.enter");
  const bool stopped = app->interrupt();
  CoreTopologyTrace::write("progress.interrupt_flag.leave");
  if (stopped)
  {
    interrupted = true;
    PRINT_ERROR("Topology calculation interrupted.\n");
    return false;
  }
  return true;
}
