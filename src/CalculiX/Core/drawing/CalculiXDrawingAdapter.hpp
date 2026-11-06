#ifndef CALCULIXDRAWINGADAPTER_HPP
#define CALCULIXDRAWINGADAPTER_HPP

#include "CoreDrawTypes.hpp"
#include "Framework/Drawing/DrawingTypes.hpp"
#include "Framework/Drawing/DrawingSymbols.hpp"

class CalculiXDrawingAdapter
{
public:
  static DrawingEntityKey key(DrawEntityKey entity, int stepId = -1);
  static bool selection(const DrawingSelection& input, CalculiXDrawSelection& output);
  static bool component(int dof, DrawingSymbolComponent& output);
  static bool displacement(DrawingGeometry& geometry, std::vector<double> point, int dof, std::string color, double size);
  static bool equationNode(DrawingGeometry& geometry, std::vector<double> point, int dof, std::string color, double size);
};
#endif
