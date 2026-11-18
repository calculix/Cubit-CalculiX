#pragma once
#include <vector>
#include "MeshQualityMetric.hpp"
#include "Framework/Core/units/QuantityUnit.hpp"
#include "Framework/Reporting/ReportConfiguration.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
class MeshQualityPresentation
{
public:
  MeshQualityPresentation();
  MeshQualityMetric metric;
  SimUnits::QuantityUnit quantity;
  bool prepare(const MeshQualityMetric& configured, const std::string& coordinateUnit,
    const ReportConfiguration& configuration, const std::vector<double>& nativeValues,
    ReportProgress* progress, ReportProgressStage stage, std::string& error);
  const std::vector<double>& values(const std::vector<double>& nativeValues) const;
private:
  bool converted = false;
  std::vector<double> samples;
};
