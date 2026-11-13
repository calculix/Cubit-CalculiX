#include <QPen>
#include "PostprocessorCaptureRenderer.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorTriad.hpp"
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationPainter.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotRendering.hpp"
#include <QGuiApplication>
#include <QThread>
#include <QBuffer>
#include <QImage>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QOpenGLContext>
#include <QPointer>
#include <vtkRenderWindow.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkCamera.h>
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkAxis.h>
#include <vtkWindowToImageFilter.h>
#include <vtkImageData.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkActorCollection.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <utility>
namespace
{
  void vtk_error(vtkObject*,unsigned long,void* client,void* message)
  {
    auto& error = *static_cast<std::string*>(client);
    if (error.empty())
    {
      error = message ? static_cast<const char*>(message) : "VTK capture failed.";
    }
  }
  template<std::size_t N> QJsonArray numbers(const std::array<double,N>& values)
  {
    QJsonArray result;
    for (double value : values)
    {
      result.append(value);
    }
    return result;
  }
  struct RestoreContext
  {
    QPointer<QOpenGLContext> context = QOpenGLContext::currentContext();
    QSurface* surface = context ? context->surface() : nullptr;
    RestoreContext()
    {
      if (context)
      {
        context->doneCurrent();
      }
    }
    ~RestoreContext()
    {
      if (context && surface)
      {
        context->makeCurrent(surface);
      }
    }
  };
}
bool PostprocessorCaptureRenderer::render(const Postprocessor::CaptureSnapshot& snapshot,
  const std::shared_ptr<PostprocessorApplicationInterface>& frozen,QByteArray& png,
  QJsonObject& provenance,std::string& error) const
{
  using namespace Postprocessor;
  error.clear();
  auto* app = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
  if (!app || QCoreApplication::closingDown() || QThread::currentThread() != app->thread() || !frozen ||
      !valid_capture(snapshot.request,error))
  {
    if (error.empty())
    {
      error = "Capture requires the live GUI application thread.";
    }
    return false;
  }
  const auto& request = snapshot.request;
  const double scale = std::min(static_cast<double>(request.width)/std::max(64,snapshot.reference_size[0]),
    static_cast<double>(request.height)/std::max(64,snapshot.reference_size[1]));
  RestoreContext restore;
  vtkNew<vtkCallbackCommand> failures;
  failures->SetClientData(&error);
  failures->SetCallback(vtk_error);
  vtkNew<vtkRenderWindow> window;
  auto* gl = vtkOpenGLRenderWindow::SafeDownCast(window);
  if (!gl)
  {
    error = "The host has no initialized OpenGL capture backend.";
    return false;
  }
  window->AddObserver(vtkCommand::ErrorEvent,failures);
  window->SetOffScreenRendering(1);
  window->SetShowWindow(false);
  window->SetAlphaBitPlanes(1);
  window->SetMultiSamples(0);
  window->SetSize(request.width,request.height);
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  renderer->AddObserver(vtkCommand::ErrorEvent,failures);
  window->AddRenderer(renderer);
  auto manifest = snapshot.provenance;
  manifest.insert("reference_size",QJsonArray{snapshot.reference_size[0],snapshot.reference_size[1]});
  std::unique_ptr<PostprocessorScene> scene;
  std::unique_ptr<PostprocessorTriad> triad;
  vtkSmartPointer<vtkContextView> context;
  vtkSmartPointer<vtkChartXY> chart;
  std::vector<EvaluatedAnnotation> annotations;
  std::optional<double> marker;
  if (request.target == CaptureTarget::View)
  {
    scene = std::make_unique<PostprocessorScene>(renderer,snapshot.target_id);
    if (!scene->synchronize(*frozen,error))
    {
      return false;
    }
    std::array<double,6> bounds;
    if (!frozen->visible_bounds(snapshot.target_id,bounds,error))
    {
      if (error.empty())
      {
        error = "The view contains no visible mesh to capture.";
      }
      return false;
    }
    const auto& view = frozen->session().views.at(snapshot.target_id);
    CameraDefinition camera;
    if (view.camera)
    {
      camera = *view.camera;
    }
    else if (!scene->prepare_camera_fit(bounds,camera,error))
    {
      return false;
    }
    scene->apply_camera(camera);
    const auto& options = snapshot.options;
    const std::array<double,3> white = {1,1,1};
    scene->apply_appearance(request.background == CaptureBackground::Transparent ? white : options.backgroundTop,
      request.background == CaptureBackground::Transparent ? white : options.backgroundBottom);
    scene->scale_capture(scale);
    manifest.insert("camera",QJsonObject{{"position",numbers(camera.position)},{"focal",numbers(camera.focal_point)},
      {"up",numbers(camera.view_up)},{"parallel",camera.parallel_projection},{"angle",camera.vertical_view_angle_degrees},
      {"scale",camera.parallel_scale}});
    manifest.insert("background_top",numbers(options.backgroundTop));
    manifest.insert("background_bottom",numbers(options.backgroundBottom));
    manifest.insert("triad",options.triadVisible);
    annotations = frozen->annotations(snapshot.target_id);
    QJsonArray records;
    for (const auto& label : annotations)
    {
      records.append(QJsonObject{{"name",QString::fromUtf8(label.name.c_str())},{"text",QString::fromUtf8(label.text.c_str())},
        {"position",numbers(label.position)},{"anchor",label.anchor ? QJsonValue(numbers(*label.anchor)) : QJsonValue(QJsonValue::Null)}});
    }
    manifest.insert("labels",records);
    triad = std::make_unique<PostprocessorTriad>(window);
    triad->update(renderer->GetActiveCamera(),options.triadVisible,
      request.background == CaptureBackground::Transparent ? white : options.backgroundBottom,
      request.width,request.height,scale);
    auto* actors = renderer->GetActors();
    actors->InitTraversal();
    while (auto* actor = actors->GetNextActor())
    {
      auto* property = actor->GetProperty();
      property->SetPointSize(static_cast<float>(property->GetPointSize()*scale));
      property->SetLineWidth(static_cast<float>(property->GetLineWidth()*scale));
    }
  }
  else
  {
    const auto found = snapshot.session.plots.find(snapshot.target_id);
    PlotData values;
    if (found == snapshot.session.plots.end() || !frozen->plot_data(snapshot.target_id,values,error))
    {
      if (error.empty())
      {
        error = "The plot is unavailable.";
      }
      return false;
    }
    context = vtkSmartPointer<vtkContextView>::New();
    window->RemoveRenderer(renderer);
    context->SetRenderWindow(window);
    renderer = vtkOpenGLRenderer::SafeDownCast(context->GetRenderer());
    if (!renderer)
    {
      error = "The host context view did not provide an OpenGL renderer.";
      return false;
    }
    renderer->AddObserver(vtkCommand::ErrorEvent,failures);
    chart = vtkSmartPointer<vtkChartXY>::New();
    context->GetScene()->AddItem(chart);
    populate_plot_chart(chart,found->second,values,scale);
    chart->SetShowLegend(request.legend.value_or(PlotDefaults::legend_visible));
    const auto& background = PlotDefaults::background;
    renderer->SetBackground(background[0],background[1],background[2]);
    if (request.marker.value_or(CaptureDefaults::marker_visible))
    {
      marker = plot_frame_marker(snapshot.session,found->second,values);
    }
    manifest.insert("legend",request.legend.value_or(PlotDefaults::legend_visible));
    manifest.insert("marker",marker ? QJsonValue(*marker) : QJsonValue(QJsonValue::Null));
  }
  if (request.background == CaptureBackground::Transparent)
  {
    renderer->GradientBackgroundOff();
    renderer->SetBackgroundAlpha(0);
  }
  window->Render();
  if (!error.empty() || !gl->GetInitialized())
  {
    if (error.empty())
    {
      error = "Cannot initialize the private OpenGL capture context.";
    }
    return false;
  }
  if (chart)
  {
    std::array<double,2> x, y;
    chart->GetAxis(vtkAxis::BOTTOM)->GetRange(x.data());
    chart->GetAxis(vtkAxis::LEFT)->GetRange(y.data());
    manifest.insert("resolved_axes",QJsonObject{{"x",numbers(x)},{"y",numbers(y)}});
  }
  vtkNew<vtkWindowToImageFilter> pixels;
  pixels->AddObserver(vtkCommand::ErrorEvent,failures);
  pixels->SetInput(window);
  pixels->SetInputBufferTypeToRGBA();
  pixels->ReadFrontBufferOff();
  pixels->ShouldRerenderOff();
  pixels->Update();
  auto* result = pixels->GetOutput();
  const auto* size = result->GetDimensions();
  if (!error.empty() || size[0] != request.width || size[1] != request.height ||
      result->GetScalarType() != VTK_UNSIGNED_CHAR || result->GetNumberOfScalarComponents() != 4)
  {
    if (error.empty())
    {
      error = "VTK returned invalid RGBA capture pixels.";
    }
    return false;
  }
  QImage image(request.width,request.height,QImage::Format_RGBA8888);
  if (image.isNull())
  {
    error = "Cannot allocate capture pixels.";
    return false;
  }
  for (int row = 0; row < request.height; ++row)
  {
    std::memcpy(image.scanLine(request.height-1-row),result->GetScalarPointer(0,row,0),static_cast<std::size_t>(request.width)*4);
  }
  image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  QPainter painter(&image);
  painter.setRenderHint(QPainter::Antialiasing);
  if (request.target == CaptureTarget::View)
  {
    paint_result_annotations(painter,renderer,annotations,image.size(),1,scale);
  }
  else if (const auto line = plot_cursor_line(chart,marker,request.height,1))
  {
    const auto& color = PlotDefaults::cursor_color;
    painter.setPen(QPen(QColor(color[0],color[1],color[2]),scale,Qt::DashLine));
    painter.drawLine(*line);
  }
  painter.end();
  const auto metadata = QJsonDocument(manifest).toJson(QJsonDocument::Compact);
  if (metadata.size() > CaptureDefaults::maximum_manifest_bytes)
  {
    error = "Capture provenance exceeds 4 MiB.";
    return false;
  }
  image.setText("CubitSim.Capture",QString::fromUtf8(metadata));
  QByteArray candidate;
  QBuffer buffer(&candidate);
  if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer,"PNG") ||
      candidate.size() > CaptureDefaults::maximum_png_bytes)
  {
    error = "Cannot encode capture PNG within the 32 MiB image limit.";
    return false;
  }
  png = std::move(candidate);
  provenance = std::move(manifest);
  return true;
}
