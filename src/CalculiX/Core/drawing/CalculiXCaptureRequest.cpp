#include "CalculiXCaptureRequest.hpp"

CalculiXCaptureRequest::CalculiXCaptureRequest()
  : content(CalculiXCaptureContent::Setup)
{
  image.width = 1920;
  image.height = 1080;
  image.view = DrawingCaptureView::Isometric;
  image.background = {1, 1, 1};
  labels.color = "black";
  labels.selectedOnly = false;
}
