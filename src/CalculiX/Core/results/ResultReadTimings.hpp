#ifndef RESULTREADTIMINGS_HPP
#define RESULTREADTIMINGS_HPP

#include "CubitMessage.hpp"
#include <chrono>
#include <cstdint>

namespace ResultReadTimings
{
using Clock = std::chrono::steady_clock;

inline std::uint64_t elapsed(Clock::time_point begin)
{
  return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - begin).count());
}

struct Phases
{
  std::uint64_t scan_ns = 0;
  std::uint64_t wait_ns = 0;
  std::uint64_t commit_ns = 0;
  std::uint64_t finalize_ns = 0;
  // Sum of worker elapsed times, not process CPU time.
  std::uint64_t worker_parse_ns = 0;
  std::uint64_t worker_derived_ns = 0;
};

inline double milliseconds(std::uint64_t value)
{
  return static_cast<double>(value) / 1000000.0;
}

inline void print(const char* reader, const Phases& phases)
{
  PRINT_INFO("%s reading phases [ms]: scan/header %.3f, wait %.3f, commit %.3f, finalize %.3f.\n",
      reader, milliseconds(phases.scan_ns), milliseconds(phases.wait_ns),
      milliseconds(phases.commit_ns), milliseconds(phases.finalize_ns));
  PRINT_INFO("%s worker totals [ms, aggregate]: numeric parsing %.3f, derived calculations %.3f.\n",
      reader, milliseconds(phases.worker_parse_ns), milliseconds(phases.worker_derived_ns));
}
}

#endif // RESULTREADTIMINGS_HPP
