#include "CoreMeshElementMapping.hpp"
#include "ElementType.h"
#include <cstddef>

namespace
{
  template<std::size_t N>
  int map_side(const int (&mapping)[N], int side)
  {
    if (side < 1 || side > static_cast<int>(N))
    {
      return -1;
    }

    return mapping[side - 1];
  }
}

int CoreMeshElementMapping::get_ccx_element_side(int element_type, int side)
{
  static const int hex[] = {3, 4, 5, 6, 1, 2};
  static const int tetra[] = {2, 3, 4, 1};
  static const int tri[] = {1, 2, 3, 5, 4};
  static const int trishell[] = {2, 1, 3, 4, 5};
  static const int shell[] = {2, 1, 3, 4, 5, 6};
  static const int quad[] = {1, 2, 3, 4, 6, 5};
  static const int wedge[] = {3, 4, 5, 1, 2};

  if (element_type >= 39 && element_type <= 43)
  {
    return map_side(hex, side);
  }

  if (element_type >= 28 && element_type <= 33)
  {
    return map_side(tetra, side);
  }

  if (element_type >= 11 && element_type <= 14)
  {
    return map_side(tri, side);
  }

  if (element_type >= 15 && element_type <= 18)
  {
    return map_side(trishell, side);
  }

  if (element_type >= 19 && element_type <= 22)
  {
    return map_side(shell, side);
  }

  if (element_type >= 23 && element_type <= 27)
  {
    return map_side(quad, side);
  }

  if (element_type == WEDGE || element_type == WEDGE6 || element_type == WEDGE15)
  {
    return map_side(wedge, side);
  }

  return side;
}
