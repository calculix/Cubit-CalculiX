#include "PostprocessorReportImageProvider.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportImage.hpp"
#include <utility>
PostprocessorReportImageProvider::PostprocessorReportImageProvider(std::weak_ptr<PostprocessorApplicationInterface> active)
  : application(std::move(active))
{
}
bool PostprocessorReportImageProvider::collect(const ReportImageRequest&,ReportDocument&,std::string& error)
{
  // User snapshots are inserted explicitly and preserved by Report Core.
  error.clear();
  return true;
}
bool PostprocessorReportImageProvider::render(const ReportImageRequest& request,ReportImage& output,std::string& error)
{
  const auto active = application.lock();
  if (!active || request.provider != "postprocessor" || !request.visual)
  {
    error = "Postprocessor capture requires a view or plot reference and the application.";
    return false;
  }
  Postprocessor::CaptureDefinition definition;
  definition.target = request.visual->kind == ReportVisualKind::View ? Postprocessor::CaptureTarget::View : Postprocessor::CaptureTarget::Plot;
  definition.name = request.visual->reference;
  if (request.width != 0 || request.height != 0)
  {
    definition.width = request.width;
    definition.height = request.height;
  }
  for (const auto& entry : request.settings.values)
  {
    if (entry.first.compare(0,8,"capture.") != 0)
    {
      continue;
    }
    const auto key = entry.first.substr(8);
    if (key == "background")
    {
      if (entry.second != "current" && entry.second != "white" && entry.second != "transparent")
      {
        error = "Capture background must be current, white or transparent.";
        return false;
      }
      definition.background = entry.second == "white" ? Postprocessor::CaptureBackground::White :
        entry.second == "transparent" ? Postprocessor::CaptureBackground::Transparent : Postprocessor::CaptureBackground::Current;
    }
    else
    {
      auto* flag = key == "legend" ? &definition.legend : key == "labels" ? &definition.labels :
        key == "triad" ? &definition.triad : key == "marker" ? &definition.marker : nullptr;
      if (!flag || (entry.second != "on" && entry.second != "off"))
      {
        error = "Invalid capture option: " + entry.first;
        return false;
      }
      *flag = entry.second == "on";
    }
  }
  QByteArray png;
  ReportImage image;
  if (!active->render_capture(definition,png,image.provenance,error))
  {
    return false;
  }
  image.id = "postprocessor." + image.provenance.value("id").toString().toStdString();
  image.caption = request.visual->reference;
  image.media_type = "image/png";
  image.base64_data = png.toBase64().toStdString();
  output = std::move(image);
  error.clear();
  return true;
}
