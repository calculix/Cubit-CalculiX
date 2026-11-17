#pragma once
#include "AnimationExportDefinition.hpp"
#include <QTemporaryDir>
#include <memory>
namespace Framework::Media
{
  class CUBITSIM_MEDIA_EXPORT AnimationOutput
  {
  public:
    AnimationOutput();
    ~AnimationOutput();
    bool prepare(const AnimationExportDefinition& definition,QString& error);
    QString frames_directory() const;
    QString encoded_file(AnimationFormat format) const;
    bool publish(const AnimationExportDefinition& definition,QString& error);
    bool cleanup();
  private:
    std::unique_ptr<QTemporaryDir> temporary;
  };
}
