#include <utility>
#include "PostprocessorApplicationInterface.hpp"
#include <algorithm>
void PostprocessorApplicationInterface::set_capture_renderer(CaptureRenderer renderer,CaptureSizer sizer)
{
  captureRenderer = std::move(renderer);
  captureSizer = std::move(sizer);
}
std::array<int,2> PostprocessorApplicationInterface::capture_reference_size(
  Postprocessor::CaptureTarget target,const std::string& id) const
{
  const auto size = captureSizer ? captureSizer(target,id) :
    std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
  return size[0] >= 64 && size[1] >= 64 ? size :
    std::array<int,2>{Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
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
bool PostprocessorApplicationInterface::capture_file(const Postprocessor::CaptureDefinition& request,
  const std::string& path,std::string& error) const
{
  QByteArray png;
  QJsonObject provenance;
  return render_capture(request,png,provenance,error) && Postprocessor::write_capture_png(path,png,error);
}
