#include "CoreTopologyLevelSetQuadrature.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include <cmath>
#include <utility>

namespace
{
bool report_error(const std::string& message, std::string* error)
{
  if (error)
  {
    *error = message;
  }
  else
  {
    PRINT_ERROR("%s\n", message.c_str());
  }
  return false;
}
}

CoreTopologyLevelSetQuadrature::CoreTopologyLevelSetQuadrature(std::size_t budget_bytes, const CoreTopologyLevelSetQuadrature* reference_rules)
  : budget_bytes(budget_bytes), reference_rules(reference_rules)
{
}

bool CoreTopologyLevelSetQuadrature::acquire(const std::string& calculix_type, int cubit_type, int subdivisions, const std::function<bool()>& poll, const Rule*& rule, std::string* error)
{
  rule = nullptr;
  const auto count = ElementInterpolation::node_count(calculix_type);
  if (!ElementInterpolation::matches_cubit_type(calculix_type, cubit_type) ||
      subdivisions < 1 || subdivisions > 32 || (subdivisions & (subdivisions - 1)) != 0)
  {
    return report_error("Invalid level-set reference quadrature request.", error);
  }
  if (poll && !poll())
  {
    return false;
  }
  const Key key(calculix_type, cubit_type, subdivisions);
  if (reference_rules)
  {
    rule = reference_rules->find(calculix_type, cubit_type, subdivisions);
    if (rule)
    {
      ++hits;
    }
    else
    {
      requests.insert(key);
      ++fallbacks;
    }
    return true;
  }
  const auto found = rules.find(key);
  if (found != rules.end())
  {
    rule = &found->second;
    ++hits;
    return true;
  }

  // Bounded subdivisions/node counts above keep this arithmetic in range.
  const auto cells = static_cast<std::size_t>(subdivisions);
  const std::size_t point_count = 27 * cells * cells * cells;
  const std::size_t point_bytes = sizeof(Point) + count * (sizeof(double) + sizeof(ElementInterpolation::Coordinates));
  const std::size_t remaining = budget_bytes - bytes;
  if (point_count > remaining / point_bytes)
  {
    ++fallbacks;
    return true;
  }

  if (building)
  {
    return report_error("Level-set reference quadrature construction cannot be reentered.", error);
  }
  const QScopedValueRollback<bool> guard(building, true);
  Rule candidate;
  candidate.points.reserve(point_count);
  std::size_t candidate_bytes = candidate.points.capacity() * sizeof(Point);
  if (candidate_bytes > remaining)
  {
    ++fallbacks;
    return true;
  }
  const double g = std::sqrt(3.0 / 5.0);
  const double points[3] = {-g, 0.0, g};
  const double weights[3] = {5.0 / 9.0, 8.0 / 9.0, 5.0 / 9.0};
  const bool tet = calculix_type == "C3D4" || calculix_type == "C3D10";
  const bool wedge = calculix_type == "C3D6" || calculix_type == "C3D15";
  const double half = 0.5 / subdivisions;
  for (int z = 0; z < subdivisions; ++z)
  {
    for (int y = 0; y < subdivisions; ++y)
    {
      for (int x = 0; x < subdivisions; ++x)
      {
        if (poll && !poll())
        {
          return false;
        }
        for (int k = 0; k < 3; ++k)
        {
          for (int j = 0; j < 3; ++j)
          {
            for (int i = 0; i < 3; ++i)
            {
              const double u = (x + 0.5) / subdivisions + half * points[i];
              const double v = (y + 0.5) / subdivisions + half * points[j];
              const double w = (z + 0.5) / subdivisions + half * points[k];
              ElementInterpolation::Coordinates point{};
              double mapping = 8.0;
              if (tet)
              {
                point = {{u, (1.0 - u) * v, (1.0 - u) * (1.0 - v) * w}};
                mapping = (1.0 - u) * (1.0 - u) * (1.0 - v);
              }
              else if (wedge)
              {
                point = {{u, (1.0 - u) * v, 2.0 * w - 1.0}};
                mapping = 2.0 * (1.0 - u);
              }
              else
              {
                point = {{2.0 * u - 1.0, 2.0 * v - 1.0, 2.0 * w - 1.0}};
              }
              Point sample;
              if (!ElementInterpolation::evaluate_reference(calculix_type, point, sample.values, sample.gradients, error))
              {
                return false;
              }
              sample.reference_weight = half * half * half * weights[i] * weights[j] * weights[k] * mapping;
              if (!std::isfinite(sample.reference_weight) || sample.reference_weight <= 0.0)
              {
                return report_error("Invalid level-set reference quadrature weight.", error);
              }
              const std::size_t dynamic_bytes = sample.values.capacity() * sizeof(double) +
                sample.gradients.capacity() * sizeof(ElementInterpolation::Coordinates);
              if (dynamic_bytes > remaining - candidate_bytes)
              {
                ++fallbacks;
                return true;
              }
              candidate_bytes += dynamic_bytes;
              candidate.points.push_back(std::move(sample));
            }
          }
        }
      }
    }
  }
  if (poll && !poll())
  {
    return false;
  }
  const auto inserted = rules.emplace(key, std::move(candidate));
  bytes += candidate_bytes;
  ++builds;
  rule = &inserted.first->second;
  return true;
}

const CoreTopologyLevelSetQuadrature::Rule* CoreTopologyLevelSetQuadrature::find(const std::string& calculix_type, int cubit_type, int subdivisions) const
{
  const auto found = rules.find(Key(calculix_type, cubit_type, subdivisions));
  return found == rules.end() ? nullptr : &found->second;
}

const std::set<CoreTopologyLevelSetQuadrature::Key>& CoreTopologyLevelSetQuadrature::requested_rules() const
{
  return requests;
}

std::size_t CoreTopologyLevelSetQuadrature::hit_count() const
{
  return hits;
}

std::size_t CoreTopologyLevelSetQuadrature::build_count() const
{
  return builds;
}

std::size_t CoreTopologyLevelSetQuadrature::fallback_count() const
{
  return fallbacks;
}

std::size_t CoreTopologyLevelSetQuadrature::retained_bytes() const
{
  return bytes;
}
