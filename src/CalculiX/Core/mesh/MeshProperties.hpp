#ifndef CALCULIX_MESH_PROPERTIES_HPP
#define CALCULIX_MESH_PROPERTIES_HPP

#include <cstddef>
#include <string>

struct MeshProperties
{
  int id = -1;
  std::string name;
  std::string elementType;
  std::string calculixElementType;
  std::size_t nodeCount = 0;
  std::size_t elementCount = 0;
  std::size_t sideCount = 0;
};

#endif
