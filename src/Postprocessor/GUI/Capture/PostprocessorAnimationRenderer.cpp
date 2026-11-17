#include "PostprocessorAnimationRenderer.hpp"
#include "PostprocessorCaptureRenderer.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
PostprocessorAnimationRenderer::PostprocessorAnimationRenderer() = default;
PostprocessorAnimationRenderer::~PostprocessorAnimationRenderer() = default;
bool PostprocessorAnimationRenderer::start(const Postprocessor::AnimationCapturePlan& requested,
  const std::shared_ptr<PostprocessorApplicationInterface>& frozen,std::string& error)
{
  auto plan = std::make_shared<Postprocessor::AnimationCapturePlan>(requested);
  QString message;
  const bool started = job.start(plan->output,[plan,frozen](std::size_t index,QImage& image,QString& failure)
  {
    Postprocessor::CaptureSnapshot frame;
    std::string error;
    QJsonObject provenance;
    if (!frozen->load_animation_sample(*plan,index,frame,error) ||
        !PostprocessorCaptureRenderer().render_image(frame,frozen,image,provenance,error))
    {
      failure = QString::fromStdString(error);
      return false;
    }
    return true;
  },[path = plan->output.path](bool succeeded,bool cancelled,const QString& failure)
  {
    if (succeeded)
    {
      PRINT_INFO("Animation saved to %s\n",path.toUtf8().constData());
      if (!failure.isEmpty())
      {
        PRINT_WARNING("%s\n",failure.toUtf8().constData());
      }
    }
    else if (cancelled && failure.isEmpty())
    {
      PRINT_INFO("Animation export cancelled.\n");
    }
    else
    {
      PRINT_ERROR("Animation export failed: %s\n",failure.toUtf8().constData());
    }
  },message);
  error = message.toStdString();
  return started;
}
void PostprocessorAnimationRenderer::cancel()
{
  job.cancel();
}
bool PostprocessorAnimationRenderer::active() const
{
  return job.active();
}
std::string PostprocessorAnimationRenderer::status() const
{
  return job.status().toStdString();
}
