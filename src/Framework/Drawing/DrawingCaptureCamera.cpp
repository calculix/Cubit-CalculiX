#include "DrawingCaptureCamera.hpp"
#include "CubitInterface.hpp"
#include "GView.hpp"
#include "GWindowFactory.hpp"
#include <QJsonArray>
#include <QCoreApplication>
#include <QThread>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <algorithm>
#include <cmath>
#include <limits>

DrawingCaptureCamera::DrawingCaptureCamera() = default;

bool DrawingCaptureCamera::valid() const
{
  const auto finite = [](double value)
  {
    return std::isfinite(value);
  };
  double distance = 0, cross = 0;
  for (int axis = 0; axis < 3; ++axis)
  {
    const double delta = position[axis] - focalPoint[axis];
    distance += delta * delta;
    const int next = (axis + 1) % 3, last = (axis + 2) % 3;
    const double value = (position[next] - focalPoint[next]) * up[last] - (position[last] - focalPoint[last]) * up[next];
    cross += value * value;
  }
  return available && std::all_of(position.begin(), position.end(), finite) &&
    std::all_of(focalPoint.begin(), focalPoint.end(), finite) && std::all_of(up.begin(), up.end(), finite) &&
    std::all_of(windowCenter.begin(), windowCenter.end(), finite) && std::isfinite(distance) && distance > 0 &&
    std::isfinite(cross) && cross > 0 && finite(parallelScale) && parallelScale > 0 && finite(viewAngle) && viewAngle > 0 && viewAngle < 180;
}

QJsonObject DrawingCaptureCamera::toJson() const
{
  return {{"available", available}, {"position", QJsonArray{position[0], position[1], position[2]}},
    {"focal", QJsonArray{focalPoint[0], focalPoint[1], focalPoint[2]}}, {"up", QJsonArray{up[0], up[1], up[2]}},
    {"parallel", parallel}, {"horizontal", horizontalAngle}, {"scale", parallelScale}, {"angle", viewAngle},
    {"center", QJsonArray{windowCenter[0], windowCenter[1]}}};
}

bool DrawingCaptureCamera::fromJson(const QJsonObject& object)
{
  DrawingCaptureCamera candidate;
  const auto read = [&](const char* key, auto& values)
  {
    const auto array = object.value(key).toArray();
    if (array.size() != static_cast<int>(values.size()))
    {
      return false;
    }
    for (int index = 0; index < array.size(); ++index)
    {
      if (!array[index].isDouble())
      {
        return false;
      }
      values[index] = array[index].toDouble();
    }
    return true;
  };
  if (!object.value("available").isBool() || !object.value("parallel").isBool() || !object.value("horizontal").isBool() ||
      !object.value("scale").isDouble() || !object.value("angle").isDouble() || !read("position", candidate.position) ||
      !read("focal", candidate.focalPoint) || !read("up", candidate.up) || !read("center", candidate.windowCenter))
  {
    return false;
  }
  candidate.available = object.value("available").toBool();
  candidate.parallel = object.value("parallel").toBool();
  candidate.horizontalAngle = object.value("horizontal").toBool();
  candidate.parallelScale = object.value("scale").toDouble();
  candidate.viewAngle = object.value("angle").toDouble();
  if (candidate.available && !candidate.valid())
  {
    return false;
  }
  *this = candidate;
  return true;
}

bool DrawingCaptureCamera::current(DrawingCaptureCamera& output, std::string& error)
{
  auto* app = QCoreApplication::instance();
  if (app == nullptr || QCoreApplication::closingDown() || QThread::currentThread() != app->thread())
  {
    error = "Reading the workspace camera requires the live Cubit application thread.";
    return false;
  }
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    error = "No Cubit graphics viewport is available for capture.";
    return false;
  }
  // Cubit's getters identify the active view. Match its camera rather than
  // assuming the first Showviz window is the user's active graphics window.
  const auto from = CubitInterface::get_view_from();
  const auto at = CubitInterface::get_view_at();
  const auto up = CubitInterface::get_view_up();
  double magnitude = 1;
  for (int axis = 0; axis < 3; ++axis)
  {
    magnitude += from[axis] * from[axis] + at[axis] * at[axis];
  }
  vtkCamera* source = nullptr;
  double closest = std::numeric_limits<double>::max();
  for (const auto& view : factory->views())
  {
    if (!view || !view->get_renderer())
    {
      continue;
    }
    auto* camera = view->get_renderer()->GetActiveCamera();
    double distance = 0;
    for (int axis = 0; axis < 3; ++axis)
    {
      distance += std::pow(camera->GetPosition()[axis] - from[axis], 2) + std::pow(camera->GetFocalPoint()[axis] - at[axis], 2);
      distance += std::pow(camera->GetViewUp()[axis] - up[axis], 2);
    }
    if (distance < closest)
    {
      closest = distance;
      source = camera;
    }
  }
  if (source == nullptr || closest > magnitude * 1e-12)
  {
    error = "No Cubit camera is available for capture.";
    return false;
  }
  DrawingCaptureCamera candidate;
  std::copy_n(source->GetPosition(), 3, candidate.position.begin());
  std::copy_n(source->GetFocalPoint(), 3, candidate.focalPoint.begin());
  std::copy_n(source->GetViewUp(), 3, candidate.up.begin());
  std::copy_n(source->GetWindowCenter(), 2, candidate.windowCenter.begin());
  candidate.parallel = source->GetParallelProjection() != 0;
  candidate.horizontalAngle = source->GetUseHorizontalViewAngle() != 0;
  candidate.parallelScale = source->GetParallelScale();
  candidate.viewAngle = source->GetViewAngle();
  candidate.available = true;
  if (!candidate.valid())
  {
    error = "The Cubit workspace camera is invalid.";
    return false;
  }
  output = candidate;
  error.clear();
  return true;
}
