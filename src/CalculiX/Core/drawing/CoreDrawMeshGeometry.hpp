#ifndef COREDRAWMESHGEOMETRY_HPP
#define COREDRAWMESHGEOMETRY_HPP

#include "CoreDrawGeometry.hpp"

class CoreDrawMeshGeometry
{
public:
  static bool append(const std::string& entity, int id, bool wireframe, const std::string& color, DrawGeometry& geometry);
};
#endif
