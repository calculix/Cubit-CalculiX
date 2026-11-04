#ifndef ELEMENTNODEORDERING_HPP
#define ELEMENTNODEORDERING_HPP

#include <cstddef>

namespace ElementNodeOrdering
{
  /**
   * @brief Return the Cubit/Exodus source index for a CalculiX node position.
   * @param element_type Cubit ElementType value used by the exporter.
   * @param calculix_index Zero-based position in CalculiX connectivity.
   * @return Zero-based source position in Cubit/Exodus connectivity.
   * @pre The caller supplies a valid position for the element type.
   * @note calculix_nodes[i] = cubit_nodes[cubit_source_index(type, i)].
   * Reorders local connectivity positions, not global node IDs. No range
   * validation is performed. Types without an explicit permutation retain their
   * input position. Export and integration must use this same conversion.
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
