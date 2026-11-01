#ifndef ELEMENTNODEORDERING_HPP
#define ELEMENTNODEORDERING_HPP

#include <cstddef>

namespace ElementNodeOrdering
{
  /*
   * Returns the Cubit/Exodus source position for a CalculiX node position.
   *
   * Both positions are zero-based:
   *   calculix_nodes[i] = cubit_nodes[cubit_source_index(type, i)]
   *
   * element_type uses the Cubit ElementType values already used by the
   * exporter. The caller must provide valid connectivity positions.
   *
   * This changes local connectivity order, not global node IDs.
   */
  inline std::size_t cubit_source_index(int element_type, std::size_t calculix_index)
  {
    constexpr int cubit_hex20 = 44;
    constexpr int cubit_wedge15 = 52;
    constexpr int cubit_bar3 = 3;
    constexpr int cubit_beam3 = 6;
    constexpr int cubit_truss3 = 9;

    if (element_type == cubit_hex20)
    {
      if (calculix_index >= 12 && calculix_index <= 15)
      {
        return calculix_index + 4;
      }

      if (calculix_index >= 16 && calculix_index <= 19)
      {
        return calculix_index - 4;
      }
    }
    else if (element_type == cubit_wedge15)
    {
      if (calculix_index >= 9 && calculix_index <= 11)
      {
        return calculix_index + 3;
      }

      if (calculix_index >= 12 && calculix_index <= 14)
      {
        return calculix_index - 3;
      }
    }
    else if (element_type == cubit_bar3 || element_type == cubit_beam3 || element_type == cubit_truss3)
    {
      if (calculix_index == 1)
      {
        return 2;
      }

      if (calculix_index == 2)
      {
        return 1;
      }
    }

    return calculix_index;
  }
}

#endif // ELEMENTNODEORDERING_HPP
