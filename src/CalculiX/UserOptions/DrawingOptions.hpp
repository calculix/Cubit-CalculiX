#ifndef DRAWINGOPTIONS_HPP
#define DRAWINGOPTIONS_HPP

#include <string>

struct DrawingOptions
{
  bool labelsEnabled = true;
  bool labelsLoads = true;
  bool labelsBcs = true;
  bool labelsSelectedOnly = false;
  bool labelsIds = true;
  bool labelsNames = true;
  bool labelsValues = false;
  bool labelsAmplitudes = false;
  bool labelsLeaders = true;
  double labelsOffset = 0.75;
  std::string labelsColor = "black";
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
