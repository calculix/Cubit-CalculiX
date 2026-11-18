#include "MeshQualityPresentation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <cmath>
#include <utility>
MeshQualityPresentation::MeshQualityPresentation() = default;
bool MeshQualityPresentation::prepare(const MeshQualityMetric& configured, const std::string& coordinateUnit,
  const ReportConfiguration& configuration, const std::vector<double>& nativeValues,
  ReportProgress* progress, ReportProgressStage stage, std::string& error)
{
  MeshQualityPresentation candidate;
  candidate.metric = configured;
  auto units = framework_units();
  SimUnits::WorkspaceUnitContext context("report");
  if (!units.workspace_context("report", context, error))
  {
    return false;
  }
  if (configuration.units)
  {
    configuration.units->apply_to(context);
  }
  if (!units.set_workspace_context(context, error) || !units.quantity(configured.quantity, candidate.quantity, error) ||
      !units.display_unit("report", configured.quantity, candidate.metric.unit, error))
  {
    return false;
  }
  candidate.quantity.unit = candidate.metric.unit;
  std::string nativeUnit = "1";
  if (configured.quantity != "dimensionless")
  {
    if (coordinateUnit.empty())
    {
      error = "Dimensional mesh quality requires a declared native geometry unit.";
      return false;
    }
    nativeUnit = coordinateUnit;
    if (configured.quantity == "area")
    {
      nativeUnit = "(" + coordinateUnit + ")^2";
    }
    else if (configured.quantity == "volume")
    {
      nativeUnit = "(" + coordinateUnit + ")^3";
    }
  }
  const auto convert = [&](double value, const std::string& from, double& result)
  {
    double canonical = 0;
    return units.to_si(value, from, candidate.quantity.dimension, candidate.quantity.role, canonical, error) &&
      units.from_si(canonical, candidate.metric.unit, candidate.quantity.dimension, candidate.quantity.role, result, error);
  };
  if (configured.minimum)
  {
    double value = 0;
    if (!convert(*configured.minimum, configured.unit, value))
    {
      return false;
    }
    candidate.metric.minimum = value;
  }
  if (configured.maximum)
  {
    double value = 0;
    if (!convert(*configured.maximum, configured.unit, value))
    {
      return false;
    }
    candidate.metric.maximum = value;
  }
  candidate.converted = nativeUnit != candidate.metric.unit;
  if (candidate.converted)
  {
    candidate.samples.reserve(nativeValues.size());
    for (std::size_t index = 0; index < nativeValues.size(); ++index)
    {
      if (index % 2048 == 0 && !ReportProgress::advance(progress, stage,
            "Converting quality units / " + configured.label, index, nativeValues.size(), error))
      {
        return false;
      }
      double result = nativeValues[index];
      if (std::isfinite(result) && !convert(result, nativeUnit, result))
      {
        return false;
      }
      candidate.samples.push_back(result);
    }
  }
  *this = std::move(candidate);
  error.clear();
  return true;
}
const std::vector<double>& MeshQualityPresentation::values(const std::vector<double>& nativeValues) const
{
  if (converted)
  {
    return samples;
  }
  return nativeValues;
}
