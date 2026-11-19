#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include <utility>
void PostprocessorApplicationInterface::set_animation_export_services(AnimationStarter start,
  std::function<void()> cancel,std::function<bool()> active,std::function<std::string()> status)
{
  animationStarter = std::move(start);
  animationCancel = std::move(cancel);
  animationActive = std::move(active);
  animationStatus = std::move(status);
}
bool PostprocessorApplicationInterface::start_animation_capture(
  const Postprocessor::AnimationCaptureDefinition& request,std::string& error) const
{
  error.clear();
  if (!animationStarter || animation_capture_active())
  {
    error = "Animation export requires the GUI service and no active export.";
    return false;
  }
  Postprocessor::CaptureSnapshot snapshot;
  if (!Postprocessor::valid_animation_capture(request,error) || !core->capture_snapshot(request.image,snapshot,error))
  {
    return false;
  }
  snapshot.options = user_options();
  if (request.image.triad)
  {
    snapshot.options.triadVisible = *request.image.triad;
  }
  if (request.image.background == Postprocessor::CaptureBackground::White)
  {
    snapshot.options.backgroundTop = snapshot.options.backgroundBottom = {1,1,1};
  }
  snapshot.reference_size = capture_reference_size(request.image.target,snapshot.target_id);
  std::string source = core->session().active_source_id;
  if (!request.source.empty())
  {
    source = core->source_id_from_name(request.source);
  }
  Postprocessor::AnimationCapturePlan plan;
  if (!plan.prepare(std::move(snapshot),source,request,error))
  {
    return false;
  }
  auto frozen = std::make_shared<PostprocessorApplicationInterface>(providers);
  frozen->core->install_capture_snapshot(plan.snapshot);
  frozen->captureOptions = plan.snapshot.options;
  return animationStarter(plan,frozen,error);
}
void PostprocessorApplicationInterface::cancel_animation_capture() const
{
  if (animationCancel)
  {
    animationCancel();
  }
}
bool PostprocessorApplicationInterface::animation_capture_active() const
{
  return animationActive && animationActive();
}
std::string PostprocessorApplicationInterface::animation_capture_status() const
{
  if (animationStatus)
  {
    return animationStatus();
  }
  return "No animation export service is available.";
}
bool PostprocessorApplicationInterface::load_animation_sample(const Postprocessor::AnimationCapturePlan& plan,
  std::size_t index,Postprocessor::CaptureSnapshot& frame,std::string& error)
{
  if (!captureOptions)
  {
    error = "Animation samples may only be installed in a detached capture application.";
    return false;
  }
  if (!plan.sample(index,frame,error))
  {
    return false;
  }
  core->install_capture_snapshot(frame);
  return true;
}
