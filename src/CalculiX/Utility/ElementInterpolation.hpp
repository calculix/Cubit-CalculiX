#ifndef ELEMENTINTERPOLATION_HPP
#define ELEMENTINTERPOLATION_HPP

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace ElementInterpolation
{
  using Coordinates = std::array<double, 3>;

  /// Supported full-integration volume type node count; zero means unsupported.
  std::size_t node_count(const std::string& calculix_type);
  /// Validate the Cubit family and expanded node count against the CalculiX type.
  bool matches_cubit_type(const std::string& calculix_type, int cubit_type);

  /**
   * @brief Evaluate shape values and reference gradients in CalculiX node order.
   * @return False with empty outputs for unsupported types or invalid points.
   * @note Tet coordinates are barycentric components 1..3; wedge coordinates
   * are triangle components 1..2 and z in [-1,1]; hex coordinates lie in [-1,1].
   * C3D4/6/8/10/15/20 are supported. C3D8R is deliberately unsupported.
   */
  bool evaluate_reference(const std::string& calculix_type, const Coordinates& point, std::vector<double>& values, std::vector<Coordinates>& gradients);

  /**
   * @brief Evaluate a mapped point using coordinates in Cubit/Exodus node order.
   * @return False with empty values and zero determinant on invalid geometry.
   * @note Values are returned in Cubit order, matching the supplied nodal field.
   * The determinant has units of length cubed, excludes quadrature weights,
   * and must be positive at this point. This is not an all-domain validity test.
   */
  bool evaluate_physical(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, const Coordinates& point, std::vector<double>& values, double& determinant);

  /**
   * @brief Map precomputed reference interpolation to one current physical element.
   * @note Reference values/gradients use CalculiX order; output values use Cubit
   * order. Recomputes the element-specific Jacobian, preserving accumulation
   * order and subtraction of node_coordinates[0]. Inputs must not alias outputs.
   * Direct vector aliasing is rejected before clearing the aliased input.
   * @return False with empty values and zero determinant on invalid input or
   * geometry (except the alias rejection above). Output capacity is reused.
   */
  bool evaluate_physical_from_reference(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, const std::vector<double>& reference_values, const std::vector<Coordinates>& reference_gradients, std::vector<double>& values, double& determinant);

}

#endif
