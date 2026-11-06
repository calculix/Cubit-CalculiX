#ifndef COREDRAWMESHGEOMETRY_HPP
#define COREDRAWMESHGEOMETRY_HPP

#include "Framework/Drawing/DrawingGeometry.hpp"

class CoreDrawMeshGeometry
{
public:
  static bool append(const std::string& entity, int id, bool wireframe, const std::string& color, DrawingGeometry& geometry);
};
#endif
