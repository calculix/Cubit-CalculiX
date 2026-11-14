#pragma once
#include "DrawingExport.hpp"
#include <QJsonObject>
#include <array>
#include <string>

// Immutable camera values in world coordinates; never owns a live viewport.
class CUBITSIM_DRAWING_EXPORT DrawingCaptureCamera
{
public:
  DrawingCaptureCamera();
  bool available = false;
  std::array<double, 3> position = {0, 0, 1};
  std::array<double, 3> focalPoint = {0, 0, 0};
  std::array<double, 3> up = {0, 1, 0};
  bool parallel = true;
  bool horizontalAngle = false;
  double parallelScale = 1;
  double viewAngle = 30;
  std::array<double, 2> windowCenter = {0, 0};
  bool valid() const;
  QJsonObject toJson() const;
  bool fromJson(const QJsonObject& object);
  static bool current(DrawingCaptureCamera& output, std::string& error);
};
