#pragma once
#include "Postprocessor/Core/capture/AnimationCapturePlan.hpp"
#include "Framework/Media/AnimationExportJob.hpp"
class PostprocessorApplicationInterface;
class PostprocessorAnimationRenderer
{
public:
  PostprocessorAnimationRenderer();
  ~PostprocessorAnimationRenderer();
  bool start(const Postprocessor::AnimationCapturePlan& plan,
    const std::shared_ptr<PostprocessorApplicationInterface>& frozen,std::string& error);
  void cancel();
  bool active() const;
  std::string status() const;
private:
  Framework::Media::AnimationExportJob job;
};
