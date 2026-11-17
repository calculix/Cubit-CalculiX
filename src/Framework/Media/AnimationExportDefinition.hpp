#pragma once
#include "MediaExport.hpp"
#include <QString>
#include <QJsonObject>
#include <QColor>
#include <cstddef>
namespace Framework::Media
{
  enum class AnimationFormat { Png, Gif, Mp4 };
  struct CUBITSIM_MEDIA_EXPORT AnimationExportDefinition
  {
    AnimationFormat format = AnimationFormat::Gif;
    QString path;
    int width = 1920, height = 1080, fps = 30;
    std::size_t frames = 0;
    bool loop = true;
    QColor background = Qt::white;
    QJsonObject metadata;
  };
  CUBITSIM_MEDIA_EXPORT bool valid_animation_export(const AnimationExportDefinition& value,QString& error);
  CUBITSIM_MEDIA_EXPORT QString animation_format_name(AnimationFormat format);
}
