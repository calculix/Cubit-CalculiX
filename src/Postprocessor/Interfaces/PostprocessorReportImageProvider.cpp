#include <type_traits>
#include "PostprocessorReportImageProvider.hpp"
#include "PostprocessorReportResultImages.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportImage.hpp"
#include <utility>
PostprocessorReportImageProvider::PostprocessorReportImageProvider(std::weak_ptr<PostprocessorApplicationInterface> active)
  : application(std::move(active))
{
}
bool PostprocessorReportImageProvider::collect(const ReportImageRequest& request,
  ReportDocument& output, std::string& error)
{
  const auto active = application.lock();
  if (!active)
  {
    error = "The Postprocessor report image provider is unavailable.";
    return false;
  }
  return PostprocessorReportResultImages().collect(*active, request, output, error);
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
  if (request.visual->kind == ReportVisualKind::View) {
    definition.target = Postprocessor::CaptureTarget::View;
  } else {
    definition.target = Postprocessor::CaptureTarget::Plot;
  }
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
      if (entry.second == "white") {
        definition.background = Postprocessor::CaptureBackground::White;
      } else {
        if (entry.second == "transparent") {
          definition.background = Postprocessor::CaptureBackground::Transparent;
        } else {
          definition.background = Postprocessor::CaptureBackground::Current;
        }
      }
    }
    else
    {
      auto* flag = [&]() -> std::common_type_t<decltype(&definition.legend), std::common_type_t<decltype(&definition.labels), std::common_type_t<decltype(&definition.triad), std::common_type_t<decltype(&definition.marker), decltype(nullptr)>>>> {
        if (key == "legend") {
          return &definition.legend;
        } else {
          return [&]() -> std::common_type_t<decltype(&definition.labels), std::common_type_t<decltype(&definition.triad), std::common_type_t<decltype(&definition.marker), decltype(nullptr)>>> {
            if (key == "labels") {
              return &definition.labels;
            } else {
              return [&]() -> std::common_type_t<decltype(&definition.triad), std::common_type_t<decltype(&definition.marker), decltype(nullptr)>> {
                if (key == "triad") {
                  return &definition.triad;
                } else {
                  return [&]() -> std::common_type_t<decltype(&definition.marker), decltype(nullptr)> {
                    if (key == "marker") {
                      return &definition.marker;
                    } else {
                      return nullptr;
                    }
                  }();
                }
              }();
            }
          }();
        }
      }();
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
