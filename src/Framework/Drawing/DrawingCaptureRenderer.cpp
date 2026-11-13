#include "DrawingCaptureRenderer.hpp"
#include "CubitColorTable.hpp"
#include <QBuffer>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QThread>
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLRenderer.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <utility>

namespace
{
void vtkFailure(vtkObject*, unsigned long, void* client, void* message)
{
  auto& error = *static_cast<std::string*>(client);
  if (error.empty())
  {
    error = message == nullptr ? "VTK capture failed." : static_cast<const char*>(message);
  }
}

bool rgb(const std::string& name, double* color)
{
  int id = -1;
  return CubitColorTable::string_to_color_id(name.c_str(), id) && CubitColorTable::color_id_to_rgb(id, color);
}

bool validPrimitive(const DrawingPrimitive& primitive)
{
  const auto count = primitive.points.size();
  if ((primitive.kind == DrawingPrimitiveKind::Point && count != 1) ||
      (primitive.kind == DrawingPrimitiveKind::Line && count != 2) ||
      (primitive.kind == DrawingPrimitiveKind::Polygon && count < 3) ||
      (primitive.kind == DrawingPrimitiveKind::Label && (count != 1 || primitive.text.empty())))
  {
    return false;
  }
  if (primitive.kind != DrawingPrimitiveKind::Point && primitive.kind != DrawingPrimitiveKind::Line &&
      primitive.kind != DrawingPrimitiveKind::Polygon && primitive.kind != DrawingPrimitiveKind::Label)
  {
    return false;
  }
  for (const auto& point : primitive.points)
  {
    if (point.size() != 3 || !std::all_of(point.begin(), point.end(), [](double v)
        {
          return std::isfinite(v);
        }))
    {
      return false;
    }
  }
  double color[3];
  return rgb(primitive.color, color);
}

bool cameraDirection(DrawingCaptureView view, std::array<double, 3>& direction, std::array<double, 3>& up)
{
  up = {0, 0, 1};
  switch (view)
  {
    case DrawingCaptureView::Isometric: direction = {1, -1, 1}; return true;
    case DrawingCaptureView::Front: direction = {0, -1, 0}; return true;
    case DrawingCaptureView::Back: direction = {0, 1, 0}; return true;
    case DrawingCaptureView::Left: direction = {-1, 0, 0}; return true;
    case DrawingCaptureView::Right: direction = {1, 0, 0}; return true;
    case DrawingCaptureView::Top: direction = {0, 0, 1}; up = {0, 1, 0}; return true;
    case DrawingCaptureView::Bottom: direction = {0, 0, -1}; up = {0, -1, 0}; return true;
  }
  return false;
}
}

DrawingCaptureRenderer::DrawingCaptureRenderer() = default;

bool DrawingCaptureRenderer::render(const DrawingFrame& frame, const DrawingCaptureRequest& request,
  QByteArray& png, std::string& error) const
{
  error.clear();
  auto* app = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
  if (app == nullptr || QCoreApplication::closingDown() || QThread::currentThread() != app->thread())
  {
    error = "Capture requires the live Cubit GUI application thread.";
    return false;
  }
  const auto pixels = static_cast<long long>(request.width) * request.height;
  std::array<double, 3> direction, up;
  const auto normalizedColor = [](double value)
  {
    return std::isfinite(value) && value >= 0 && value <= 1;
  };
  if (request.width < 64 || request.height < 64 || request.width > 8192 || request.height > 8192 || pixels > 32000000 ||
      !std::all_of(request.background.begin(), request.background.end(), normalizedColor) ||
      !cameraDirection(request.view, direction, up))
  {
    error = "Invalid capture dimensions, camera or background. Use 64..8192 pixels per side, at most 32 million pixels.";
    return false;
  }
  // Group by kind and color: actor count does not grow with mesh element count.
  std::map<std::pair<DrawingPrimitiveKind, std::string>, std::vector<const DrawingPrimitive*>> batches;
  std::vector<const DrawingPrimitive*> labels;
  double bounds[6] = {std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()};
  bool haveGeometry = false;
  if (frame.overlayVisible)
  {
    for (const auto& entity : frame.entities)
    {
      const auto layer = frame.layers.find(entity.layer);
      if (layer == frame.layers.end() || !layer->second.enabled)
      {
        continue;
      }
      for (const auto& primitive : entity.geometry.primitives)
      {
        if (!validPrimitive(primitive))
        {
          error = "Capture scene contains invalid geometry, text or colors.";
          return false;
        }
        if (primitive.kind == DrawingPrimitiveKind::Label)
        {
          labels.push_back(&primitive);
        }
        else
        {
          batches[{primitive.kind, primitive.color}].push_back(&primitive);
          haveGeometry = true;
        }
        for (const auto& point : primitive.points)
        {
          for (std::size_t axis = 0; axis < 3; ++axis)
          {
            bounds[2 * axis] = (std::min)(bounds[2 * axis], point[axis]);
            bounds[2 * axis + 1] = (std::max)(bounds[2 * axis + 1], point[axis]);
          }
        }
      }
    }
  }
  if (!haveGeometry)
  {
    error = "The capture scene has no visible model geometry.";
    return false;
  }
  vtkNew<vtkCallbackCommand> failures;
  failures->SetClientData(&error);
  failures->SetCallback(vtkFailure);
  vtkNew<vtkRenderWindow> window;
  auto* gl = vtkOpenGLRenderWindow::SafeDownCast(window);
  if (gl == nullptr)
  {
    error = "The host has no initialized VTK OpenGL render-window backend.";
    return false;
  }
  window->AddObserver(vtkCommand::ErrorEvent, failures);
  window->SetOffScreenRendering(1);
  window->SetShowWindow(false);
  window->SetMultiSamples(0);
  window->SetSize(request.width, request.height);
  vtkNew<vtkOpenGLRenderer> renderer;
  renderer->AddObserver(vtkCommand::ErrorEvent, failures);
  renderer->SetBackground(request.background.data());
  window->AddRenderer(renderer);
  for (const auto& batch : batches)
  {
    vtkNew<vtkPoints> points;
    points->SetDataTypeToDouble();
    vtkNew<vtkCellArray> cells;
    for (const auto* primitive : batch.second)
    {
      cells->InsertNextCell(static_cast<vtkIdType>(primitive->points.size()));
      for (const auto& point : primitive->points)
      {
        cells->InsertCellPoint(points->InsertNextPoint(point.data()));
      }
    }
    vtkNew<vtkPolyData> data;
    data->SetPoints(points);
    switch (batch.first.first)
    {
      case DrawingPrimitiveKind::Point: data->SetVerts(cells); break;
      case DrawingPrimitiveKind::Line: data->SetLines(cells); break;
      case DrawingPrimitiveKind::Polygon: data->SetPolys(cells); break;
      case DrawingPrimitiveKind::Label: break;
    }
    vtkNew<vtkOpenGLPolyDataMapper> mapper;
    mapper->AddObserver(vtkCommand::ErrorEvent, failures);
    mapper->SetInputData(data);
    mapper->ScalarVisibilityOff();
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    double color[3];
    rgb(batch.first.second, color);
    actor->GetProperty()->SetColor(color);
    actor->GetProperty()->LightingOff();
    actor->GetProperty()->SetPointSize(4);
    actor->GetProperty()->SetLineWidth(1.5);
    renderer->AddActor(actor);
  }
  // Parallel projection produces reproducible engineering views. Direction
  // vectors are camera positions relative to the model center; Z is up except
  // for top/bottom. ResetCamera fits the supplied bounds to the image aspect.
  auto* camera = renderer->GetActiveCamera();
  camera->ParallelProjectionOn();
  camera->SetPosition(direction.data());
  camera->SetFocalPoint(0, 0, 0);
  camera->SetViewUp(up.data());
  renderer->ResetCamera(bounds);
  camera->SetParallelScale(camera->GetParallelScale() * 1.15);
  renderer->ResetCameraClippingRange(bounds);
  window->Render();
  if (!error.empty() || !gl->GetInitialized())
  {
    if (error.empty())
    {
      error = "Unable to create an OpenGL context for PNG capture.";
    }
    return false;
  }
  vtkNew<vtkWindowToImageFilter> pixelsFilter;
  pixelsFilter->AddObserver(vtkCommand::ErrorEvent, failures);
  pixelsFilter->SetInput(window);
  pixelsFilter->SetInputBufferTypeToRGB();
  pixelsFilter->ReadFrontBufferOff();
  pixelsFilter->ShouldRerenderOff();
  pixelsFilter->Update();
  auto* result = pixelsFilter->GetOutput();
  const auto* dimensions = result->GetDimensions();
  if (!error.empty() || dimensions[0] != request.width || dimensions[1] != request.height ||
      result->GetNumberOfScalarComponents() != 3 || result->GetScalarType() != VTK_UNSIGNED_CHAR)
  {
    if (error.empty())
    {
      error = "VTK returned an invalid capture image.";
    }
    return false;
  }
  QImage image(request.width, request.height, QImage::Format_RGB888);
  if (image.isNull())
  {
    error = "Cannot allocate capture image.";
    return false;
  }
  for (int row = 0; row < request.height; ++row)
  {
    std::memcpy(image.scanLine(request.height - 1 - row), result->GetScalarPointer(0, row, 0), request.width * 3);
  }
  // Camera-facing labels are painted after readback, independent of VTK font
  // factories. They overlay the image; no text occlusion/collision is inferred.
  QPainter painter(&image);
  painter.setRenderHint(QPainter::TextAntialiasing);
  auto font = painter.font();
  font.setPixelSize((std::max)(12, request.height / 60));
  painter.setFont(font);
  for (const auto* label : labels)
  {
    const auto& point = label->points.front();
    renderer->SetWorldPoint(point[0], point[1], point[2], 1);
    renderer->WorldToDisplay();
    const auto* position = renderer->GetDisplayPoint();
    if (!std::isfinite(position[0]) || !std::isfinite(position[1]) || position[2] < 0 || position[2] > 1)
    {
      continue;
    }
    double color[3];
    rgb(label->color, color);
    painter.setPen(QColor::fromRgbF(color[0], color[1], color[2]));
    painter.drawText(QPointF(position[0] + 3, request.height - 1 - position[1] - 3), QString::fromStdString(label->text));
  }
  painter.end();
  QByteArray candidate;
  QBuffer buffer(&candidate);
  if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG"))
  {
    error = "Unable to encode capture as PNG.";
    return false;
  }
  png = std::move(candidate);
  error.clear();
  return true;
}
