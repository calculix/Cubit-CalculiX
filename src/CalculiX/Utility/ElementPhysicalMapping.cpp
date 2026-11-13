#include "ElementPhysicalMapping.hpp"
#include "ElementNodeOrdering.hpp"
#include "CubitMessage.hpp"
#include "Eigen/Dense"
#include <cmath>

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

bool ElementPhysicalMapping::prepare(const std::string& type, int cubit_type, const std::vector<ElementInterpolation::Coordinates>& coordinates, std::string* error)
{
  count = 0;
  calculix_type.clear();
  const std::size_t candidate_count = ElementInterpolation::node_count(type);
  if (!ElementInterpolation::matches_cubit_type(type, cubit_type) || candidate_count > sources.size() || coordinates.size() != candidate_count)
  {
    return report_error("Incompatible connectivity in element physical mapping.", error);
  }
  for (const auto& point : coordinates)
  {
    for (double value : point)
    {
      if (!std::isfinite(value))
      {
        return report_error("Non-finite element coordinates in interpolation.", error);
      }
    }
  }
  for (std::size_t node = 0; node < candidate_count; ++node)
  {
    const auto source = ElementNodeOrdering::cubit_source_index(cubit_type, node);
    if (source >= candidate_count)
    {
      return report_error("Invalid node ordering in element physical mapping.", error);
    }
    sources[node] = source;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      const double offset = coordinates[source][axis] - coordinates[0][axis];
      if (!std::isfinite(offset))
      {
        return report_error("Non-finite coordinate offset in element physical mapping.", error);
      }
      coordinate_offsets[node][axis] = offset;
    }
  }
  calculix_type = type;
  count = candidate_count;
  return true;
}

bool ElementPhysicalMapping::map_reference(const std::vector<double>& reference_values, const std::vector<ElementInterpolation::Coordinates>& reference_gradients, std::vector<double>& values, double& determinant, std::string* error) const
{
  if (&values == &reference_values)
  {
    return report_error("Reference interpolation inputs must not alias output values.", error);
  }
  values.clear();
  determinant = 0.0;
  if (count == 0 || reference_values.size() != count || reference_gradients.size() != count)
  {
    return report_error("Unprepared physical mapping or incompatible reference data.", error);
  }
  Eigen::Matrix3d jacobian = Eigen::Matrix3d::Zero();
  for (std::size_t node = 0; node < count; ++node)
  {
    if (!std::isfinite(reference_values[node]))
    {
      return report_error("Non-finite reference values in element interpolation.", error);
    }
    for (double gradient : reference_gradients[node])
    {
      if (!std::isfinite(gradient))
      {
        return report_error("Non-finite reference gradients in element interpolation.", error);
      }
    }
    for (std::size_t row = 0; row < 3; ++row)
    {
      const double coordinate = coordinate_offsets[node][row];
      for (std::size_t column = 0; column < 3; ++column)
      {
        jacobian(row, column) += coordinate * reference_gradients[node][column];
      }
    }
  }
  const double value = jacobian.determinant();
  if (!std::isfinite(value) || value <= 0.0)
  {
    return report_error("Invalid Jacobian determinant in element interpolation.", error);
  }
  values.resize(count);
  for (std::size_t node = 0; node < count; ++node)
  {
    values[sources[node]] = reference_values[node];
  }
  determinant = value;
  return true;
}

bool ElementPhysicalMapping::evaluate_at(const ElementInterpolation::Coordinates& point, std::vector<double>& values, double& determinant, std::string* error) const
{
  values.clear();
  determinant = 0.0;
  if (count == 0)
  {
    return report_error("Element physical mapping has not been prepared.", error);
  }
  std::vector<double> reference;
  std::vector<ElementInterpolation::Coordinates> gradients;
  if (!ElementInterpolation::evaluate_reference(calculix_type, point, reference, gradients, error))
  {
    return false;
  }
  return map_reference(reference, gradients, values, determinant, error);
}
