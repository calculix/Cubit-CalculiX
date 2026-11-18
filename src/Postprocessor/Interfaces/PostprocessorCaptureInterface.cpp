#include <type_traits>
#include <utility>
#include "PostprocessorApplicationInterface.hpp"
#include "Framework/Reporting/ReportImage.hpp"
#include <algorithm>
void PostprocessorApplicationInterface::set_capture_renderer(CaptureRenderer renderer,CaptureSizer sizer)
{
  captureRenderer = std::move(renderer);
  captureSizer = std::move(sizer);
}
std::array<int,2> PostprocessorApplicationInterface::capture_reference_size(
  Postprocessor::CaptureTarget target,const std::string& id) const
{
  const auto size = [&]() -> std::common_type_t<decltype(captureSizer(target,id)), decltype(std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height})> {
    if (captureSizer) {
      return captureSizer(target,id);
    } else {
      return std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
    }
  }();
  if (size[0] >= 64 && size[1] >= 64) {
    return size;
  } else {
    return std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
  }
}
bool PostprocessorApplicationInterface::render_capture(const Postprocessor::CaptureDefinition& request,
  QByteArray& png,QJsonObject& provenance,std::string& error) const
{
  error.clear();
  if (!captureRenderer)
  {
    error = "PNG capture requires the active Cubit GUI rendering service.";
    return false;
  }
  Postprocessor::CaptureSnapshot snapshot;
  if (!capture_snapshot(request, snapshot, error))
  {
    return false;
  }
  auto frozen = std::make_shared<PostprocessorApplicationInterface>(providers);
  frozen->core.install_capture_snapshot(snapshot);
  frozen->captureOptions = snapshot.options;
  QByteArray candidate;
  QJsonObject manifest;
  if (!captureRenderer(snapshot,frozen,candidate,manifest,error))
  {
    return false;
  }
  png = std::move(candidate);
  provenance = std::move(manifest);
  return true;
}
bool PostprocessorApplicationInterface::capture_snapshot(const Postprocessor::CaptureDefinition& request,
  Postprocessor::CaptureSnapshot& output, std::string& error) const
{
  Postprocessor::CaptureSnapshot snapshot;
  if (!core.capture_snapshot(request,snapshot,error))
  {
    return false;
  }
  snapshot.options = user_options();
  if (request.triad)
  {
    snapshot.options.triadVisible = *request.triad;
  }
  if (request.background == Postprocessor::CaptureBackground::White)
  {
    snapshot.options.backgroundTop = snapshot.options.backgroundBottom = {1,1,1};
  }
  snapshot.reference_size = capture_reference_size(request.target,snapshot.target_id);
  output = std::move(snapshot);
  error.clear();
  return true;
}
bool PostprocessorApplicationInterface::capture_file(const Postprocessor::CaptureDefinition& request,
  const std::string& path,std::string& error) const
{
  QByteArray png;
  QJsonObject provenance;
  return render_capture(request,png,provenance,error) && Postprocessor::write_capture_png(path,png,error);
}

const SimUnits::WorkspaceUnitContext* PostprocessorApplicationInterface::captured_units() const noexcept
{
  return core.captured_units();
}

bool PostprocessorApplicationInterface::render_report_result(const Postprocessor::CaptureSnapshot& snapshot,
  ReportImage& output, std::string& error) const
{
  if (!captureRenderer)
  {
    error = "Result images require the Cubit GUI capture renderer.";
    return false;
  }
  auto frozen = std::make_shared<PostprocessorApplicationInterface>(providers);
  frozen->core.install_capture_snapshot(snapshot);
  frozen->captureOptions = snapshot.options;
  QByteArray png;
  ReportImage candidate;
  if (!captureRenderer(snapshot, frozen, png, candidate.provenance, error))
  {
    return false;
  }
  candidate.media_type = "image/png";
  candidate.base64_data = png.toBase64().toStdString();
  output = std::move(candidate);
  error.clear();
  return true;
}
