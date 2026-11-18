#include "MeshQualitySettings.hpp"
#include "MeshQualityDefaults.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QLocale>
#include <cmath>
#include <utility>
MeshQualitySettings::MeshQualitySettings() = default;
bool MeshQualitySettings::decode(const ReportConfiguration& configuration,
  MeshQualitySettings& output, std::string& error)
{
  MeshQualitySettings candidate;
  const auto flag = [&](const std::string& key, bool initial, bool& value)
  {
    const auto found = configuration.values.find(key);
    value = initial;
    if (found == configuration.values.end())
    {
      return true;
    }
    if (found->second != "true" && found->second != "false")
    {
      error = "Expected true or false for " + key;
      return false;
    }
    value = found->second == "true";
    return true;
  };
  if (!flag("mesh.quality.enabled", true, candidate.enabled) ||
      !flag("mesh.quality.statistics", true, candidate.statistics) ||
      !flag("mesh.quality.images", true, candidate.images) ||
      !flag("mesh.quality.failed_images", true, candidate.failed_images) ||
      !flag("mesh.quality.element_values", false, candidate.element_values))
  {
    return false;
  }
  for (auto metric : MeshQualityDefaults::metrics())
  {
    bool selected = false;
    if (!flag(metric.key() + ".enabled", metric.enabled_by_default, selected))
    {
      return false;
    }
    for (const auto& item : {std::make_pair(".minimum", &metric.minimum), std::make_pair(".maximum", &metric.maximum)})
    {
      const auto found = configuration.values.find(metric.key() + item.first);
      if (found == configuration.values.end())
      {
        continue;
      }
      if (found->second.empty())
      {
        item.second->reset();
        continue;
      }
      bool valid = false;
      const double value = QLocale::c().toDouble(QString::fromStdString(found->second), &valid);
      if (!valid || !std::isfinite(value))
      {
        error = "Invalid quality limit: " + metric.key() + item.first;
        return false;
      }
      *item.second = value;
    }
    const auto unit = configuration.values.find(metric.key() + ".unit");
    if (unit != configuration.values.end())
    {
      metric.unit = unit->second;
    }
    SimUnits::QuantityUnit descriptor;
    double ignored = 0;
    if (!framework_units().quantity(metric.quantity, descriptor, error) ||
        !framework_units().to_si(0, metric.unit, descriptor.dimension, descriptor.role, ignored, error))
    {
      error = "Invalid quality limits unit: " + metric.key() + " / " + error;
      return false;
    }
    if (metric.minimum && metric.maximum && *metric.minimum > *metric.maximum)
    {
      error = "Quality minimum exceeds maximum: " + metric.key();
      return false;
    }
    if (selected)
    {
      candidate.metrics.push_back(std::move(metric));
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
