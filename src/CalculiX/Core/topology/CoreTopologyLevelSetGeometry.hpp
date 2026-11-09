#ifndef CORETOPOLOGYLEVELSETGEOMETRY_HPP
#define CORETOPOLOGYLEVELSETGEOMETRY_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <array>
#include <functional>
#include <string>
#include <vector>

class CoreTopologyLevelSetQuadrature;

class CoreTopologyLevelSetGeometry
{
public:
  /**
   * @brief Query one current Cubit volume element without retaining a mesh copy.
   * @param element_id Global Cubit element ID.
   * @param[out] calculix_type Block's full-integration CalculiX type.
   * @param[out] cubit_type Cubit expanded-connectivity type.
   * @param[out] nodes Global node IDs in Cubit expanded-connectivity order.
   * @param[out] coordinates Finite coordinates in the same order.
   * @return False with cleared outputs and a Cubit error on invalid data.
   */
  static bool query_element(int element_id, std::string& calculix_type, int& cubit_type, std::vector<int>& nodes, std::vector<std::array<double, 3>>& coordinates);

  /**
   * @brief Evaluate a regularized fraction on one existing unstructured element.
   * @param calculix_type Supported full-integration CalculiX volume element type.
   * @param cubit_type Matching Cubit expanded-connectivity element type.
   * @param node_coordinates Coordinates in Cubit expanded-connectivity order.
   * @param node_values Dimensionless coefficients in [-1,1], in that same order.
   * @param cubit_volume Positive volume queried from Cubit, in length cubed.
   * @param preserved Force fraction one and derivatives zero after geometry checks.
   * @param settings Valid settings; quadrature controls are independent of solver rules.
   * @param[out] geometry Cleared on entry/failure; replaced only after convergence.
   * @param poll Optional calling-thread progress/interrupt callback; false aborts.
   * The callback owns update throttling; quadrature does not publish percentages.
   * @param quadrature Optional pass-local reference-rule cache; oversized rules
   * use the ordinary path and all physical Jacobians remain element-specific.
   * @return False with cleared geometry on invalid data, unresolved integration or interruption.
   * @note Positive phi is solid; phi=0 is the interface. Uses a compact C1
   * Heaviside with dimensionless half-width settings.interface_width. Higher-order
   * interpolation may overshoot nodal bounds; the Heaviside still lies in [0,1].
   * Composite three-point tensor Gauss rules use a Duffy map for tet/wedge.
   * Refinement compares volume, fraction and width-scaled nodal derivatives.
   * This is an error estimate, not a certified integration or element-validity
   * bound. It does not split/remesh Cubit elements or store a mesh snapshot.
   */
  static bool evaluate_element(const std::string& calculix_type, int cubit_type, const std::vector<std::array<double, 3>>& node_coordinates, const std::vector<double>& node_values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, const std::function<bool()>& poll = {}, CoreTopologyLevelSetQuadrature* quadrature = nullptr);
};

#endif
