#ifndef DRAWINGOPTIONS_HPP
#define DRAWINGOPTIONS_HPP

#include <string>

struct DrawingOptions
{
  double sizeFactor;
  bool overlayVisible;
  bool boundaryConditions;
  bool loads;
  bool constraints;
  bool orientations;
  std::string hoverColor;
  std::string selectionColor;
};

#endif
