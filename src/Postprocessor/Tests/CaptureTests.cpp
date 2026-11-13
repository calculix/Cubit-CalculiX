#include "CaptureTests.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QTemporaryDir>
#include <QFile>
#include <QBuffer>
#include <QImage>
#include <iostream>
int capture_cases(const std::shared_ptr<PostprocessorApplicationInterface>& app)
{
  using namespace Postprocessor;
  int failures = 0;
  const auto check = [&](bool value,const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  CaptureDefinition request;
  check(valid_capture(request,error),"Core capture defaults are invalid");
  request.width = 8192;
  request.height = 8192;
  check(!valid_capture(request,error),"Capture exceeded pixel budget");
  request = {};
  request.marker = false;
  check(!valid_capture(request,error),"View accepted plot-only marker option");
  request = {};
  request.target = CaptureTarget::Plot;
  request.labels = false;
  check(!valid_capture(request,error),"Plot accepted view-only labels option");
  request = {};
  QByteArray result("unchanged");
  QJsonObject manifest{{"untouched",true}};
  check(!app->render_capture(request,result,manifest,error) && result == "unchanged" && manifest.value("untouched").toBool(),
    "Unavailable renderer changed output");
  const auto view = app->session().active_view_id;
  const auto source = app->session().active_source_id;
  const auto data = app->result_data(source);
  app->set_capture_renderer([&](const CaptureSnapshot& snapshot,const std::shared_ptr<PostprocessorApplicationInterface>& frozen,
    QByteArray& png,QJsonObject& provenance,std::string&)
  {
    check(snapshot.session.active_view_id == view && frozen.get() != app.get(),"Capture did not create a private application");
    check(frozen->result_data(source) == data,"Capture duplicated or lost the immutable result handle");
    check(!frozen->session().views.at(view).selection.entity,"Capture retained transient selection highlighting");
    std::string local;
    check(frozen->set_source_visible("Model",false,local),"Cannot mutate private capture state");
    check(app->session().representations.begin()->second.visible,"Private capture mutation changed live visibility");
    png = "PNG fixture";
    provenance = snapshot.provenance;
    return true;
  });
  check(app->render_capture(request,result,manifest,error) && result == "PNG fixture" && manifest.contains("sources"),
    "Capture boundary failed to publish its frozen result");
  app->set_capture_renderer({});
  QImage pixels(64,64,QImage::Format_ARGB32);
  pixels.fill(Qt::white);
  QByteArray png;
  QBuffer buffer(&png);
  buffer.open(QIODevice::WriteOnly);
  pixels.save(&buffer,"PNG");
  QTemporaryDir directory;
  const auto path = directory.filePath("capture.png");
  check(write_capture_png(path.toStdString(),png,error),"Cannot publish PNG capture");
  check(!write_capture_png(path.toStdString(),QByteArray("replacement"),error),"Capture overwrote an existing file");
  QFile file(path);
  check(file.open(QIODevice::ReadOnly) && file.readAll() == png,"No-overwrite failure damaged existing PNG");
  return failures;
}
