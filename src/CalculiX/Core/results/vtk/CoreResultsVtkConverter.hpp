#ifndef CORERESULTSVTKCONVERTER_HPP
#define CORERESULTSVTKCONVERTER_HPP

#include "CoreResultsVtkStructs.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <string>

class CoreResultsFrd;
class CoreResultsDat;

// Sequential caller-side elapsed milliseconds, including worker waits. These
// are wall-clock phases, not summed worker/CPU times. Only complete builds
// publish measurements; planning and file export are measured by CoreResults.
struct CoreResultsVtkTimings
{
  double geometry_ms = 0.0;
  double fields_ms = 0.0;
  double ip_displacement_ms = 0.0;
  double tables_ms = 0.0;
  double finalize_ms = 0.0;
};

class CoreResultsVtkConverter
{
public:
  /** Consume a fresh plan and stable readers, without querying the live model.
   * poll runs only on the caller thread; false requests cancellation.
   * All tasks are joined before return. Output is replaced only after complete
   * conversion/validation. An empty error with false means user cancellation.
   * workers must be resolved to a positive count by the core/caller.
   */
  bool build(const CoreResultsFrd& frd, const CoreResultsDat& dat, const CoreResultsVtkPlan& plan, std::atomic<bool>& cancelled, const std::function<bool(double)>& poll, std::shared_ptr<const SimResults::VtkResultsData>& output, std::string& error, CoreResultsVtkTimings* timings = nullptr) const;
};

#endif // CORERESULTSVTKCONVERTER_HPP
