#include "ElementIntegration.hpp"
#include "ElementInterpolation.hpp"
#include "CubitMessage.hpp"
#include <cmath>

bool ElementIntegration::get_rule(const std::string& calculix_type, std::vector<IntegrationPoint>& points)
{
  points.clear();

  const double g2 = 0.577350269189626;
  const double g3 = 0.774596669241483;

  if (calculix_type == "C3D8R")
  {
    points.push_back({{{0.0, 0.0, 0.0}}, 8.0});
  }
  else if (calculix_type == "C3D20R")
  {
    return get_rule("C3D8", points);
  }
  else if (calculix_type == "C3D4")
  {
    points.push_back({{{0.25, 0.25, 0.25}}, 1.0 / 6.0});
  }
  else if (calculix_type == "C3D10")
  {
    const double a = 0.138196601125011;
    const double b = 0.585410196624968;

    points.push_back({{{a, a, a}}, 1.0 / 24.0});
    points.push_back({{{b, a, a}}, 1.0 / 24.0});
    points.push_back({{{a, b, a}}, 1.0 / 24.0});
    points.push_back({{{a, a, b}}, 1.0 / 24.0});
  }
  else if (calculix_type == "C3D6")
  {
    points.push_back({{{1.0 / 3.0, 1.0 / 3.0, -g2}}, 0.5});
    points.push_back({{{1.0 / 3.0, 1.0 / 3.0, g2}}, 0.5});
  }
  else if (calculix_type == "C3D15")
  {
    const double z[3] = {-g3, 0.0, g3};
    const double wz[3] = {5.0 / 9.0, 8.0 / 9.0, 5.0 / 9.0};
    const double triangle[3][2] = {{1.0 / 6.0, 1.0 / 6.0}, {2.0 / 3.0, 1.0 / 6.0}, {1.0 / 6.0, 2.0 / 3.0}};

    for (std::size_t layer = 0; layer < 3; ++layer)
    {
      for (std::size_t point = 0; point < 3; ++point)
      {
        points.push_back({{{triangle[point][0], triangle[point][1], z[layer]}}, wz[layer] / 6.0});
      }
    }
  }
  else if (calculix_type == "C3D8" || calculix_type == "C3D20")
  {
    const bool quadratic = calculix_type == "C3D20";
    const std::size_t count = quadratic ? 3 : 2;
    const double positions[3] = {quadratic ? -g3 : -g2, quadratic ? 0.0 : g2, g3};
    const double weights[3] = {quadratic ? 5.0 / 9.0 : 1.0, quadratic ? 8.0 / 9.0 : 1.0, 5.0 / 9.0};

    // CalculiX numbering: xi changes fastest, then eta, then zeta.
    for (std::size_t z = 0; z < count; ++z)
    {
      for (std::size_t y = 0; y < count; ++y)
      {
        for (std::size_t x = 0; x < count; ++x)
        {
          points.push_back({{{positions[x], positions[y], positions[z]}}, weights[x] * weights[y] * weights[z]});
        }
      }
    }
  }
  else
  {
    PRINT_ERROR("Unsupported CalculiX integration type '%s'.\n", calculix_type.c_str());
    return false;
  }

  return true;
}

bool ElementIntegration::shape_functions(const std::string& calculix_type, const Coordinates& reference, std::vector<double>& values)
{
  const std::string type = calculix_type == "C3D8R" ? "C3D8" : calculix_type == "C3D20R" ? "C3D20" : calculix_type;
  std::vector<Coordinates> gradients;
  // Reuse the shared interpolation kernel; reduced integration changes the
  // evaluation point, not the geometric shape functions.
  return ElementInterpolation::evaluate_reference(type, reference, values, gradients);
}

bool ElementIntegration::interpolate_point(const std::string& calculix_type, int point_id, const std::vector<Coordinates>& nodal_values, Coordinates& position)
{
  std::vector<IntegrationPoint> rule;
  if (!get_rule(calculix_type, rule))
  {
    return false;
  }
  if (point_id <= 0 || static_cast<std::size_t>(point_id) > rule.size())
  {
    PRINT_ERROR("Invalid integration point %d for '%s'.\n", point_id, calculix_type.c_str());
    return false;
  }
  std::vector<double> shape;
  if (!shape_functions(calculix_type, rule[point_id - 1].coordinates, shape))
  {
    return false;
  }
  if (shape.size() != nodal_values.size())
  {
    PRINT_ERROR("Incorrect nodal value count for '%s'.\n", calculix_type.c_str());
    return false;
  }
  Coordinates result{};
  for (std::size_t node = 0; node < shape.size(); ++node)
  {
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      if (!std::isfinite(nodal_values[node][axis]))
      {
        PRINT_ERROR("Non-finite nodal value in integration-point interpolation.\n");
        return false;
      }
      result[axis] += shape[node] * nodal_values[node][axis];
    }
  }
  for (double value : result)
  {
    if (!std::isfinite(value))
    {
      PRINT_ERROR("Non-finite interpolated integration point.\n");
      return false;
    }
  }
  position = result;
  return true;
}

bool ElementIntegration::calculate_weights(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, std::map<int, double>& weights)
{
  weights.clear();
  if (!ElementInterpolation::matches_cubit_type(calculix_type, cubit_type) || node_coordinates.size() != ElementInterpolation::node_count(calculix_type))
  {
    PRINT_ERROR("Incompatible connectivity for CalculiX type '%s' and Cubit type %d.\n", calculix_type.c_str(), cubit_type);
    return false;
  }
  std::vector<IntegrationPoint> points;
  if (!get_rule(calculix_type, points))
  {
    return false;
  }
  std::map<int, double> collected;
  for (std::size_t point = 0; point < points.size(); ++point)
  {
    double determinant = 0.0;
    std::vector<double> values;
    if (!ElementInterpolation::evaluate_physical(calculix_type, cubit_type, node_coordinates, points[point].coordinates, values, determinant))
    {
      return false;
    }
    const double weight = points[point].weight * determinant;
    if (!std::isfinite(weight) || weight <= 0.0)
    {
      PRINT_ERROR("Invalid physical integration weight for '%s' at integration point %d.\n", calculix_type.c_str(), static_cast<int>(point + 1));
      return false;
    }
    collected.emplace(static_cast<int>(point + 1), weight);
  }
  weights.swap(collected);
  return true;
}
