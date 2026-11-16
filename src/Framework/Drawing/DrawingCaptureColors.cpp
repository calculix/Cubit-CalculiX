#include "DrawingCaptureColors.hpp"
#include "CubitColor.hpp"
#include "CubitInterface.hpp"
#include "GWindowFactory.hpp"
#include "GView.hpp"
#include "GModelSet.hpp"
#include "GSimModel.hpp"
#include "GSimInterface.hpp"
#include <QCoreApplication>
#include <QThread>
#include <QString>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <cmath>
#include <limits>
#include <utility>

DrawingCaptureColors::DrawingCaptureColors() = default;

bool DrawingCaptureColors::encode(const CubitColor& color, std::string& output, std::string& error)
{
  if (color.alpha == 0)
  {
    error = "Native block color is unset; its displayed color must be resolved.";
    return false;
  }
  output = QString("#%1%2%3")
    .arg(static_cast<unsigned int>(color.red), 2, 16, QLatin1Char('0'))
    .arg(static_cast<unsigned int>(color.green), 2, 16, QLatin1Char('0'))
    .arg(static_cast<unsigned int>(color.blue), 2, 16, QLatin1Char('0')).toStdString();
  error.clear();
  return true;
}

bool DrawingCaptureColors::block(MeshOutputGroup* block, std::string& output, std::string& error)
{
  auto* application = QCoreApplication::instance();
  if (!block || !application || QThread::currentThread() != application->thread())
  {
    error = "Resolving a displayed block color requires the Cubit GUI thread.";
    return false;
  }
  auto* factory = GWindowFactory::get_window_factory();
  if (!factory)
  {
    error = "No native graphics view is available to resolve the block color.";
    return false;
  }
  const auto from = CubitInterface::get_view_from();
  const auto at = CubitInterface::get_view_at();
  const auto up = CubitInterface::get_view_up();
  GView* selected = nullptr;
  double closest = (std::numeric_limits<double>::max)();
  for (const auto& view : factory->views())
  {
    if (!view || !view->get_renderer() || !view->get_model_set())
    {
      continue;
    }
    auto* camera = view->get_renderer()->GetActiveCamera();
    double distance = 0.0;
    for (int axis = 0; axis < 3; ++axis)
    {
      const double position = camera->GetPosition()[axis] - from[axis];
      const double focal = camera->GetFocalPoint()[axis] - at[axis];
      const double vertical = camera->GetViewUp()[axis] - up[axis];
      distance += position * position + focal * focal + vertical * vertical;
    }
    if (std::isfinite(distance) && distance < closest)
    {
      closest = distance;
      selected = view.get();
    }
  }
  if (!selected)
  {
    error = "No native graphics model is available to resolve the block color.";
    return false;
  }
  std::string resolved;
  for (auto* model : selected->get_model_set()->models())
  {
    auto* simulation = dynamic_cast<GSimModel*>(model);
    if (!simulation || !simulation->sim_interface())
    {
      continue;
    }
    const auto color = simulation->sim_interface()->get_color(block);
    std::string value;
    std::string reason;
    if (!encode(color, value, reason))
    {
      continue;
    }
    if (!resolved.empty() && resolved != value)
    {
      error = "Native graphics models disagree about the displayed block color.";
      return false;
    }
    resolved = std::move(value);
  }
  if (resolved.empty())
  {
    error = "Native graphics did not resolve the unset block color; capture will not substitute black.";
    return false;
  }
  output = std::move(resolved);
  error.clear();
  return true;
}
