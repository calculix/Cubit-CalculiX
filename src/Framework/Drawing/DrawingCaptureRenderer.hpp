#pragma once
#include "DrawingExport.hpp"
#include "DrawingCaptureRequest.hpp"
#include "DrawingTypes.hpp"
#include <QByteArray>

class CUBITSIM_DRAWING_EXPORT DrawingCaptureRenderer
{
public:
  DrawingCaptureRenderer();
  // Host/GUI thread only. Uses a private render window, never a live viewport.
  // Width/height: 64..8192, at most 32 million pixels. Output is atomic.
  bool render(const DrawingFrame& frame, const DrawingCaptureRequest& request,
    QByteArray& png, std::string& error) const;
};
