#include "MeshQualityStatistics.hpp"
#include <algorithm>
#include <cmath>
MeshQualityStatistics::MeshQualityStatistics() = default;
bool MeshQualityStatistics::outside(double value, const MeshQualityMetric& metric)
{
  return std::isfinite(value) && ((metric.minimum && value < *metric.minimum) ||
    (metric.maximum && value > *metric.maximum));
}
bool MeshQualityStatistics::calculate(const std::vector<std::int64_t>& ids,
  const std::vector<double>& values, const MeshQualityMetric& metric, std::string& error)
{
  if (ids.size() != values.size())
  {
    error = "Quality sample and identity counts differ.";
    return false;
  }
  MeshQualityStatistics candidate;
  double m2 = 0;
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    const double value = values[index];
    if (!std::isfinite(value))
    {
      ++candidate.unevaluable;
      continue;
    }
    if (candidate.evaluated == 0 || value < candidate.minimum ||
        (value == candidate.minimum && ids[index] < candidate.minimum_id))
    {
      candidate.minimum = value;
      candidate.minimum_id = ids[index];
    }
    if (candidate.evaluated == 0 || value > candidate.maximum ||
        (value == candidate.maximum && ids[index] < candidate.maximum_id))
    {
      candidate.maximum = value;
      candidate.maximum_id = ids[index];
    }
    ++candidate.evaluated;
    const double delta = value - candidate.mean;
    candidate.mean += delta / static_cast<double>(candidate.evaluated);
    m2 += delta * (value - candidate.mean);
    if (outside(value, metric))
    {
      ++candidate.failed;
    }
  }
  if (candidate.evaluated != 0)
  {
    candidate.standard_deviation = std::sqrt(std::max(0.0, m2 / candidate.evaluated));
  }
  if (!std::isfinite(candidate.mean) || !std::isfinite(candidate.standard_deviation))
  {
    error = "Quality statistics exceed the finite numeric range.";
    return false;
  }
  *this = candidate;
  error.clear();
  return true;
}
