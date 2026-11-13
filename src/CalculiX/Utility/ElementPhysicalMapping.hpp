#ifndef ELEMENTPHYSICALMAPPING_HPP
#define ELEMENTPHYSICALMAPPING_HPP

#include "ElementInterpolation.hpp"
#include <array>
#include <cstddef>
#include <string>
#include <vector>

/**
 * @brief Validated coordinates and node ordering for one physical element.
 * @note prepare copies at most 20 coordinate offsets, not a Cubit mesh/model.
 * Offsets use CalculiX node order and the same Cubit node-zero origin as the
 * existing interpolation kernel. Point/refinement accumulation order is unchanged.
 * No affine assumption is made: every point recomputes and checks its Jacobian.
 * One element worker owns the object. Preparation/repreparation must not overlap
 * evaluation. A failed prepare invalidates the object; no stale mapping is usable.
 * Non-null error destinations defer messages to the caller, without Cubit calls.
 */
class ElementPhysicalMapping
{
public:
  bool prepare(const std::string& calculix_type, int cubit_type, const std::vector<ElementInterpolation::Coordinates>& coordinates, std::string* error = nullptr);

  /**
   * @brief Map CalculiX-ordered reference values/gradients to Cubit-ordered values.
   * @return False with empty values/zero determinant on failure. Direct aliasing
   * of reference_values and values is rejected before modifying either output,
   * preserving the existing public interpolation contract.
   * @note Reference sizes/finiteness and the physical determinant remain checked
   * at every point. Positive determinant is a sampled check, not a global proof.
   */
  bool map_reference(const std::vector<double>& reference_values, const std::vector<ElementInterpolation::Coordinates>& reference_gradients, std::vector<double>& values, double& determinant, std::string* error = nullptr) const;

  /// Ordinary reference evaluation followed by the same prepared physical map.
  bool evaluate_at(const ElementInterpolation::Coordinates& point, std::vector<double>& values, double& determinant, std::string* error = nullptr) const;

private:
  std::string calculix_type;
  std::size_t count = 0;
  std::array<std::size_t, 20> sources{};
  std::array<ElementInterpolation::Coordinates, 20> coordinate_offsets{};
};

#endif
