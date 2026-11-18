#pragma once
#include <cstddef>
#include "MeshQualityMetric.hpp"
#include <cstdint>
#include <vector>
class MeshQualityStatistics
{
public:
  MeshQualityStatistics();
  std::size_t evaluated = 0, unevaluable = 0, failed = 0;
  double minimum = 0, maximum = 0, mean = 0, standard_deviation = 0;
  std::int64_t minimum_id = 0, maximum_id = 0;
  static bool outside(double value, const MeshQualityMetric& metric);
  // Finite samples; population variance M2/n. Equal extrema use the lowest ID.
  bool calculate(const std::vector<std::int64_t>& ids, const std::vector<double>& values,
    const MeshQualityMetric& metric, std::string& error);
};
