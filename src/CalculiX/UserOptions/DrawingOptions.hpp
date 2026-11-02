#ifndef DRAWINGOPTIONS_HPP
#define DRAWINGOPTIONS_HPP

#include <string>

struct DrawingOptions
{
  double sizeFactor = 1.0;
  bool overlayVisible = true;
  bool boundaryConditions = false;
  bool loads = false;
  bool constraints = false;
  bool orientations = false;
  std::string hoverColor = "red";
  std::string selectionColor = "yellow";
};

#endif
