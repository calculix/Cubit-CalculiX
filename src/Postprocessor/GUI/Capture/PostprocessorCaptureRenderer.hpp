#ifndef POSTPROCESSORCAPTURERENDERER_HPP
#define POSTPROCESSORCAPTURERENDERER_HPP
#include "Postprocessor/Core/capture/CaptureDefinition.hpp"
class PostprocessorApplicationInterface;
class PostprocessorCaptureRenderer
{
public:
  bool render(const Postprocessor::CaptureSnapshot& snapshot,
    const std::shared_ptr<PostprocessorApplicationInterface>& frozen,
    QByteArray& png,QJsonObject& provenance,std::string& error) const;
};
#endif
