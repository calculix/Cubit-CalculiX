#ifndef CORETOPOLOGYPROGRESS_HPP
#define CORETOPOLOGYPROGRESS_HPP

#include <chrono>
#include <cstddef>

class ProgressTool;

/**
 * @brief Manage one Cubit progress-bar scope on the calling thread.
 *
 * Percentages represent completed elements, not estimated execution time.
 * GUI updates and interrupt polling are throttled to 200 ms.
 * Initial display and successful completion bypass that throttle.
 * Destruction closes the bar, including on failure or interruption.
 * Scopes must not overlap because Cubit provides one shared progress tool.
 * A missing progress service leaves calculations usable without a bar.
 */
class CoreTopologyProgress
{
public:
  CoreTopologyProgress(const char* title, std::size_t total);
  ~CoreTopologyProgress();

  CoreTopologyProgress(const CoreTopologyProgress&) = delete;
  CoreTopologyProgress& operator=(const CoreTopologyProgress&) = delete;

  /// Service progress during expensive work without advancing the element count.
  bool poll();
  /// Count one successfully processed element and service progress if due.
  bool advance();
  /// Check interruption and display 100% after successful validation.
  bool finish();

private:
  bool check_interrupt();

  ProgressTool* progress = nullptr;
  std::size_t total = 0;
  std::size_t completed = 0;
  bool interrupted = false;
  std::chrono::steady_clock::time_point previous;
};

#endif
