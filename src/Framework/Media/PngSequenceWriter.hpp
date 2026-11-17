#pragma once
#include "AnimationExportDefinition.hpp"
#include <QImage>
namespace Framework::Media
{
  class CUBITSIM_MEDIA_EXPORT PngSequenceWriter
  {
  public:
    static QString frame_path(const QString& directory,std::size_t index);
    static bool write_frame(const QString& directory,std::size_t index,const QImage& image,QString& error);
    static bool write_manifest(const QString& directory,const AnimationExportDefinition& definition,QString& error);
  };
}
