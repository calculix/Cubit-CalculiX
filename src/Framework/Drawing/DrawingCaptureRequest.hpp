#pragma once
#include "DrawingExport.hpp"
#include <array>

enum class DrawingCaptureView
{
  Isometric, Front, Back, Left, Right, Top, Bottom
};

// Renderer inputs. The owning workspace supplies defaults and dimensions.
class CUBITSIM_DRAWING_EXPORT DrawingCaptureRequest
{
public:
  DrawingCaptureRequest();
  int width = 0;
  int height = 0;
  DrawingCaptureView view = DrawingCaptureView::Isometric;
  std::array<double, 3> background = {1, 1, 1};
};
