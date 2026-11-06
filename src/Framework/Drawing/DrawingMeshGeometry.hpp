#ifndef CUBITSIM_DRAWINGMESHGEOMETRY_HPP
#define CUBITSIM_DRAWINGMESHGEOMETRY_HPP

#include "DrawingExport.hpp"

#include "DrawingGeometry.hpp"

class CUBITSIM_DRAWING_EXPORT DrawingMeshGeometry
{
public:
  static bool append(const std::string& entity, int id, bool wireframe, const std::string& color, DrawingGeometry& geometry);
};
#endif
