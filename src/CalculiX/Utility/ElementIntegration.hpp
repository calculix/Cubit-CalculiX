#ifndef ELEMENTINTEGRATION_HPP
#define ELEMENTINTEGRATION_HPP

#include <array>
#include <map>
#include <string>
#include <vector>

namespace ElementIntegration
{
  using Coordinates = std::array<double, 3>;

  struct IntegrationPoint
  {
    Coordinates coordinates;
    double weight;
  };

  /**
   * @brief Return reference integration points in CalculiX order.
   * @param calculix_type C3D4, C3D6, C3D8, C3D10, C3D15, or C3D20.
   * @param points Output reference coordinates and weights; index+1 is the point ID.
   * @return True for a supported rule; false with empty output otherwise.
   * @note Point counts are 1, 2, 8, 4, 9, and 27 respectively. C3D8R is unsupported.
   */
  bool get_rule(const std::string& calculix_type, std::vector<IntegrationPoint>& points);

  /**
   * @brief Calculate physical quadrature weights using shared node-order conversion.
   * @param calculix_type Supported full-integration CalculiX volume element type.
   * @param cubit_type Matching Cubit element type for the coordinate ordering.
   * @param node_coordinates Finite coordinates in Cubit/Exodus connectivity order.
   * @param weights Output map keyed by one-based CalculiX integration point ID.
   * @return True on success; false with empty output on unsupported or invalid data.
   * @note Each value is reference weight times det(J). Coordinate count/type must
   * match, and every determinant and physical weight must be finite and positive.
   */
  bool calculate_weights(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, std::map<int, double>& weights);
}

#endif // ELEMENTINTEGRATION_HPP
