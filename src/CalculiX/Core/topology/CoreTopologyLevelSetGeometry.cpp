#include "CoreTopologyLevelSetGeometry.hpp"
#include "CoreTopologyLevelSetQuadrature.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "ElementInterpolation.hpp"
#include "CubitMessage.hpp"
#include "CubitInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include <set>
#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
struct Integral
{
  double volume = 0.0;
  double solid = 0.0;
  std::vector<double> derivatives;
};

void heaviside(double phi, double width, double& value, double& derivative)
{
  if (phi <= -width)
  {
    value = 0.0;
    derivative = 0.0;
  }
  else if (phi >= width)
  {
    value = 1.0;
    derivative = 0.0;
  }
  else
  {
    const double pi = std::acos(-1.0);
    const double scaled = phi / width;
    value = std::clamp(0.5 * (1.0 + scaled + std::sin(pi * scaled) / pi), 0.0, 1.0);
    derivative = 0.5 * (1.0 + std::cos(pi * scaled)) / width;
  }
}

bool integrate(const std::string& type, int cubit_type, const std::vector<ElementInterpolation::Coordinates>& coordinates, const std::vector<double>& phi, double width, bool preserved, int subdivisions, Integral& integral, const std::function<bool()>& poll, CoreTopologyLevelSetQuadrature* quadrature)
{
  integral = {};
  integral.derivatives.assign(phi.size(), 0.0);
  const CoreTopologyLevelSetQuadrature::Rule* rule = nullptr;
  if (quadrature && !quadrature->acquire(type, cubit_type, subdivisions, poll, rule))
  {
    return false;
  }
  std::size_t sample_index = 0;
  std::vector<double> shape;
  shape.reserve(phi.size());
  const double g = std::sqrt(3.0 / 5.0);
  const double points[3] = {-g, 0.0, g};
  const double weights[3] = {5.0 / 9.0, 8.0 / 9.0, 5.0 / 9.0};
  const bool tet = type == "C3D4" || type == "C3D10";
  const bool wedge = type == "C3D6" || type == "C3D15";
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
              double determinant = 0.0;
              double reference_weight = 0.0;
              if (rule)
              {
                const auto& sample = rule->points[sample_index++];
                if (!ElementInterpolation::evaluate_physical_from_reference(type, cubit_type, coordinates, sample.values, sample.gradients, shape, determinant))
                {
                  return false;
                }
                reference_weight = sample.reference_weight;
              }
              else
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
                if (!ElementInterpolation::evaluate_physical(type, cubit_type, coordinates, point, shape, determinant))
                {
                  return false;
                }
                reference_weight = half * half * half * weights[i] * weights[j] * weights[k] * mapping;
              }
              const double weight = reference_weight * determinant;
              if (!std::isfinite(weight) || weight <= 0.0)
              {
                PRINT_ERROR("Invalid level-set geometry integration weight.\n");
                return false;
              }
              double field = 0.0;
              for (std::size_t node = 0; node < phi.size(); ++node)
              {
                field += shape[node] * phi[node];
              }
              if (!std::isfinite(field))
              {
                PRINT_ERROR("Non-finite interpolated level-set field.\n");
                return false;
              }
              double value = 1.0;
              double derivative = 0.0;
              if (!preserved)
              {
                heaviside(field, width, value, derivative);
              }
              integral.volume += weight;
              integral.solid += weight * value;
              for (std::size_t node = 0; node < phi.size(); ++node)
              {
                integral.derivatives[node] += weight * derivative * shape[node];
              }
            }
          }
        }
      }
    }
  }
  if (!std::isfinite(integral.volume) || integral.volume <= 0.0 || !std::isfinite(integral.solid))
  {
    PRINT_ERROR("Invalid integrated level-set volume.\n");
    return false;
  }
  integral.solid /= integral.volume;
  for (double& derivative : integral.derivatives)
  {
    derivative /= integral.volume;
    if (!std::isfinite(derivative))
    {
      PRINT_ERROR("Non-finite level-set fraction derivative.\n");
      return false;
    }
  }
  if (!std::isfinite(integral.solid) || integral.solid < 0.0 || integral.solid > 1.0)
  {
    PRINT_ERROR("Invalid integrated level-set solid fraction.\n");
    return false;
  }
  return true;
}

bool converged(const Integral& previous, const Integral& current, const TopologyLevelSetSettings& settings)
{
  const double relative_volume = std::abs(current.volume - previous.volume) / (std::max)(current.volume, previous.volume);
  double error = (std::max)(relative_volume, std::abs(current.solid - previous.solid));
  for (std::size_t node = 0; node < current.derivatives.size(); ++node)
  {
    error = (std::max)(error, settings.interface_width * std::abs(current.derivatives[node] - previous.derivatives[node]));
  }
  return error <= settings.integration_tolerance;
}
}

bool CoreTopologyLevelSetGeometry::evaluate_element(const std::string& calculix_type, int cubit_type, const std::vector<std::array<double, 3>>& node_coordinates, const std::vector<double>& node_values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, const std::function<bool()>& poll, CoreTopologyLevelSetQuadrature* quadrature)
{
  geometry = {};
  if (!CoreTopologyLevelSet::validate_settings(settings))
  {
    return false;
  }
  if (!ElementInterpolation::matches_cubit_type(calculix_type, cubit_type) ||
      node_coordinates.size() != ElementInterpolation::node_count(calculix_type) || node_values.size() != node_coordinates.size() ||
      !std::isfinite(cubit_volume) || cubit_volume <= 0.0)
  {
    PRINT_ERROR("Invalid level-set element geometry request.\n");
    return false;
  }
  for (double value : node_values)
  {
    if (!std::isfinite(value) || value < -1.0 || value > 1.0)
    {
      PRINT_ERROR("Level-set nodal values must be finite and in [-1,1].\n");
      return false;
    }
  }
  Integral previous;
  if (!integrate(calculix_type, cubit_type, node_coordinates, node_values, settings.interface_width, preserved, 1, previous, poll, quadrature))
  {
    return false;
  }
  int subdivisions = 1;
  for (int refinement = 0; refinement < settings.integration_max_refinements; ++refinement)
  {
    subdivisions *= 2;
    Integral current;
    if (!integrate(calculix_type, cubit_type, node_coordinates, node_values, settings.interface_width, preserved, subdivisions, current, poll, quadrature))
    {
      return false;
    }
    if (converged(previous, current, settings))
    {
      TopologyLevelSetElementGeometry candidate;
      candidate.solid_fraction = current.solid;
      candidate.integration_volume = current.volume;
      candidate.solid_volume = cubit_volume * current.solid;
      candidate.fraction_derivatives = std::move(current.derivatives);
      candidate.volume_derivatives.reserve(node_values.size());
      for (double derivative : candidate.fraction_derivatives)
      {
        const double volume_derivative = cubit_volume * derivative;
        if (!std::isfinite(volume_derivative))
        {
          PRINT_ERROR("Non-finite level-set volume derivative.\n");
          return false;
        }
        candidate.volume_derivatives.push_back(volume_derivative);
      }
      candidate.subdivisions = subdivisions;
      geometry = std::move(candidate);
      return true;
    }
    previous = std::move(current);
  }
  PRINT_ERROR("Level-set element geometry integration did not converge within %d refinements.\n", settings.integration_max_refinements);
  return false;
}

bool CoreTopologyLevelSetGeometry::query_element(int element_id, std::string& calculix_type, int& cubit_type, std::vector<int>& nodes, std::vector<std::array<double, 3>>& coordinates)
{
  calculix_type.clear();
  cubit_type = -1;
  nodes.clear();
  coordinates.clear();
  if (element_id <= 0 || !CubitInterface::get_element_exists(element_id))
  {
    PRINT_ERROR("Invalid level-set element ID %d.\n", element_id);
    return false;
  }
  CalculiXMeshInterface mesh;
  std::string entity = CubitInterface::get_element_type(element_id);
  if (entity != "hex" && entity != "tet" && entity != "wedge")
  {
    entity = mesh.get_cubit_element_type_entity(entity);
  }
  const int local_id = CubitInterface::get_element_type_id(element_id);
  const int block_id = CubitInterface::get_element_block(element_id);
  if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0 || block_id <= 0 || !mesh.check_block_exists(block_id))
  {
    PRINT_ERROR("Invalid volume type or block for level-set element %d.\n", element_id);
    return false;
  }
  const std::string type = mesh.get_ccx_element_type(block_id);
  const auto connectivity = CubitInterface::get_expanded_connectivity(entity, local_id);
  int candidate_type = -1;
  if (entity == "hex")
  {
    candidate_type = connectivity.size() == 8 ? 42 : connectivity.size() == 20 ? 44 : -1;
  }
  else if (entity == "tet")
  {
    candidate_type = connectivity.size() == 4 ? 31 : connectivity.size() == 10 ? 33 : -1;
  }
  else
  {
    candidate_type = connectivity.size() == 6 ? 50 : connectivity.size() == 15 ? 52 : -1;
  }
  if (!ElementInterpolation::matches_cubit_type(type, candidate_type))
  {
    PRINT_ERROR("Unsupported CalculiX type/connectivity for level-set element %d.\n", element_id);
    return false;
  }
  std::set<int> unique;
  std::vector<std::array<double, 3>> collected;
  for (int node : connectivity)
  {
    if (node <= 0 || !unique.insert(node).second || !CubitInterface::get_node_exists(node))
    {
      PRINT_ERROR("Invalid or duplicate node in level-set element %d.\n", element_id);
      return false;
    }
    const auto point = CubitInterface::get_nodal_coordinates(node);
    for (double value : point)
    {
      if (!std::isfinite(value))
      {
        PRINT_ERROR("Non-finite coordinates in level-set element %d.\n", element_id);
        return false;
      }
    }
    collected.push_back(point);
  }
  calculix_type = type;
  cubit_type = candidate_type;
  nodes.assign(connectivity.begin(), connectivity.end());
  coordinates.swap(collected);
  return true;
}
