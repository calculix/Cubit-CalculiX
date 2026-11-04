#ifndef ELEMENTINTEGRATION_HPP
#define ELEMENTINTEGRATION_HPP

#include <array>
#include <map>
#include <string>
#include <vector>

namespace ElementIntegration
{
  using Coordinates = std::array<double, 3>;

  /** @brief One CalculiX-ordered reference quadrature point; no Jacobian scaling is stored here. */
  struct IntegrationPoint
  {
    /// Dimensionless reference coordinates, not model-space XYZ coordinates.
    Coordinates coordinates;
    /// Dimensionless reference quadrature weight, not a physical volume weight.
    double weight;
  };

  /**
   * @brief Return reference integration points in CalculiX order.
   * @param calculix_type C3D4, C3D6, C3D8, C3D8R, C3D10, C3D15, C3D20, or C3D20R.
   * @param points Output reference coordinates and weights; index+1 is the point ID.
   * @return True for a supported rule; false with empty output otherwise.
   * @note Full-integration rules retain CCX 2.23 gauss.f numbering and weights.
   * C3D8R uses gauss3d1 (one point); C3D20R uses gauss3d2 (eight points).
   */
  bool get_rule(const std::string& calculix_type, std::vector<IntegrationPoint>& points);

  /** Evaluate geometric shape functions in CalculiX local node order.
   * Reference coordinates use the CCX convention (hex: [-1,1]^3;
   * tet: xi,eta,zeta >= 0, sum <= 1; wedge: triangle x [-1,1]).
   * Supports the same volume types as get_rule; R changes quadrature, not shape.
   * Returns false with empty output for unsupported/non-finite input.
   */
  bool shape_functions(const std::string& calculix_type, const Coordinates& reference, std::vector<double>& values);

  /** Map a one-based CCX integration-point ID to model coordinates.
   * Coordinates must already be in CalculiX order. The same interpolation can
   * be applied to nodal displacements: u_ip=sum(N_a*u_a), x_display=x_ip+s*u_ip.
   * No quadrature weight/Jacobian factor belongs in this interpolation.
   * Returns false without changing position for invalid/unsupported input.
   */
  bool interpolate_point(const std::string& calculix_type, int point_id, const std::vector<Coordinates>& nodal_values, Coordinates& position);

  /**
   * @brief Calculate physical quadrature weights using shared node-order conversion.
   * @param calculix_type Supported full-integration CalculiX volume element type.
   * @param cubit_type Matching Cubit element type for the coordinate ordering.
   * @param node_coordinates Finite coordinates in Cubit/Exodus connectivity order.
   * @param weights Output map keyed by one-based CalculiX integration point ID.
   * @return True on success; false with empty output on unsupported or invalid data.
   * @note Each value is reference weight times det(J). Coordinate count/type must
   * match, and every determinant and physical weight must be finite and positive.
   * Returned weights have units of model length cubed and already contain det(J).
   * Multiply them into the adjoint contraction once. Cubit volume-constraint
   * queries are separate; this method does not require their values to match.
   */
  bool calculate_weights(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, std::map<int, double>& weights);
}

#endif // ELEMENTINTEGRATION_HPP
