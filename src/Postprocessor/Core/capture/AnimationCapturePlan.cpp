#include "AnimationCapturePlan.hpp"
#include "../representations/ResultMesh.hpp"
#include "../views/CameraOperations.hpp"
#include "../representations/AnimationRotationCodec.hpp"
#include <QJsonArray>
#include <cmath>
#include <utility>
namespace Postprocessor
{
  bool AnimationCapturePlan::prepare(CaptureSnapshot frozen,const std::string& source,
    const AnimationCaptureDefinition& request,std::string& error)
  {
    error.clear();
    if (!valid_animation_capture(request,error))
    {
      return false;
    }
    const auto loaded = frozen.sources.find(source);
    const auto view = frozen.session.views.find(frozen.target_id);
    if (loaded == frozen.sources.end() || view == frozen.session.views.end() || !view->second.camera ||
        !CameraOperations::valid(*view->second.camera))
    {
      error = "Choose an available mesh source and fit the capture view before rendering animation.";
      return false;
    }
    AnimationCapturePlan candidate;
    candidate.source_id = source;
    for (const auto& entry : frozen.session.representations)
    {
      const auto& display = entry.second;
      std::string input = display.input.object_id;
      if (display.input.owner == OutputOwner::Feature)
      {
        const auto feature = frozen.session.filters.find(input);
        if (feature == frozen.session.filters.end())
        {
          continue;
        }
        input = feature->second.input.object_id;
      }
      if (display.view_id == frozen.target_id && display.visible && input == source)
      {
        if (!candidate.representation_id.empty() && entry.first != frozen.session.active_representation_id)
        {
          continue;
        }
        candidate.representation_id = entry.first;
      }
    }
    if (candidate.representation_id.empty())
    {
      error = "The selected source has no visible representation in the capture view.";
      return false;
    }
    auto& display = frozen.session.representations.at(candidate.representation_id);
    if (request.rotation)
    {
      display.animation.rotation = *request.rotation;
    }
    candidate.result_fps = request.result_fps;
    if (candidate.result_fps == 0)
    {
      candidate.result_fps = display.animation.frames_per_second;
    }
    const auto& data = *loaded->second;
    std::size_t last = request.last;
    if (last == 0)
    {
      last = data.increments.size();
    }
    if (request.first > data.increments.size() || last > data.increments.size())
    {
      error = "The animation frame range is outside the loaded sequence.";
      return false;
    }
    for (std::size_t frame = request.first-1; frame < last; ++frame)
    {
      if (!result_meshes(data.increments[frame].dataset.get()).empty())
      {
        candidate.frames.push_back(frame);
      }
    }
    if (candidate.frames.empty())
    {
      error = "The animation range has no displayable mesh frames.";
      return false;
    }
    candidate.output = request.output;
    candidate.output.width = request.image.width;
    candidate.output.height = request.image.height;
    const double count = std::ceil(static_cast<double>(candidate.frames.size())*request.passes*
      candidate.output.fps/candidate.result_fps);
    if (!std::isfinite(count) || count < 1 || count > 100000)
    {
      error = "Animation export must contain 1..100000 output frames.";
      return false;
    }
    candidate.output.frames = static_cast<std::size_t>(count);
    QString mediaError;
    if (!Framework::Media::valid_animation_export(candidate.output,mediaError))
    {
      error = mediaError.toStdString();
      return false;
    }
    candidate.output.metadata = frozen.provenance;
    candidate.output.metadata.insert("result_fps",candidate.result_fps);
    candidate.output.metadata.insert("rotation",animation_rotation_to_json(display.animation.rotation));
    candidate.output.metadata.insert("passes",request.passes);
    candidate.output.metadata.insert("representation",QString::fromStdString(candidate.representation_id));
    QJsonArray selected;
    for (const auto frame : candidate.frames)
    {
      selected.append(static_cast<int>(frame+1));
    }
    candidate.output.metadata.insert("result_frames",selected);
    candidate.snapshot = std::move(frozen);
    *this = std::move(candidate);
    return true;
  }
  bool AnimationCapturePlan::sample(std::size_t index,CaptureSnapshot& result,std::string& error) const
  {
    if (index >= output.frames || frames.empty())
    {
      error = "Animation sample is outside the prepared timeline.";
      return false;
    }
    auto candidate = snapshot;
    const double time = static_cast<double>(index)/output.fps;
    const auto ordinal = static_cast<std::size_t>(std::floor(time*result_fps+1e-9));
    auto& display = candidate.session.representations.at(representation_id);
    display.increment_index = frames[ordinal%frames.size()];
    const double phase = time*result_fps/static_cast<double>(frames.size());
    std::array<double,3> angles = {0,0,0};
    for (std::size_t axis = 0; axis < angles.size(); ++axis)
    {
      if (display.animation.rotation[axis].enabled)
      {
        angles[axis] = display.animation.rotation[axis].degrees*phase;
      }
    }
    auto& view = candidate.session.views.at(candidate.target_id);
    CameraDefinition camera;
    if (!CameraOperations::rotate_world(*view.camera,angles,camera,error))
    {
      return false;
    }
    view.camera = camera;
    candidate.session.active_source_id = source_id;
    candidate.session.active_representation_id = representation_id;
    candidate.provenance.insert("animation",QJsonObject{{"output_frame",static_cast<int>(index)},
      {"time_seconds",time},{"result_frame",static_cast<int>(display.increment_index+1)},{"pass_phase",phase}});
    result = std::move(candidate);
    return true;
  }
}
