#pragma once
#include "MeshQualityMetric.hpp"
#include "Framework/Reporting/ReportConfiguration.hpp"
#include <vector>
class MeshQualitySettings
{
public:
  MeshQualitySettings();
  bool enabled = true, statistics = true, images = true, failed_images = true;
  bool element_values = false;
  std::vector<MeshQualityMetric> metrics;
  static bool decode(const ReportConfiguration& configuration, MeshQualitySettings& output, std::string& error);
};
