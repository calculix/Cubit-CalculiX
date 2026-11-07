#ifndef CUBITSIM_RESOURCEATTRIBUTION_HPP
#define CUBITSIM_RESOURCEATTRIBUTION_HPP

#include "ResourceMonitor.hpp"

struct ResourceAttribution
{
  bool available = false;
  bool incomplete = false;
  bool adjusted = false;
  // Fractions of machine capacity; their sum equals the measured used fraction.
  double cubit = 0;
  double jobs = 0;
  double otherShared = 0;
  // Original measurements, never scaled. CPU uses percent; memory uses bytes.
  std::optional<double> cubitMeasured;
  double jobsMeasured = 0;
  bool jobsIncomplete = false;
};

// Approximate attribution, not an exact accounting partition. Disabled categories
// remain in otherShared. Missing data must never become a claimed zero usage.
// CPU sampling intervals can differ; memory can include shared pages repeatedly.
// When process totals exceed measured machine usage, scale only colored widths
// proportionally. Original measurements stay unchanged for tooltips/details.
ResourceAttribution memory_attribution(const ResourceSnapshot& snapshot, bool includeCubit, bool includeJobs);
ResourceAttribution cpu_attribution(const ResourceSnapshot& snapshot, bool includeCubit, bool includeJobs);

#endif
