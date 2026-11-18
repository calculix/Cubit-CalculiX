#pragma once
#include "MeshQualityMetric.hpp"
#include <vector>
class ReportTemplate;
class MeshQualityDefaults
{
public:
  MeshQualityDefaults();
  static std::vector<MeshQualityMetric> metrics();
  static void add_options(ReportTemplate& definition);
};
