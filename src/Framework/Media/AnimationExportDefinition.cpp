#include "AnimationExportDefinition.hpp"
#include <QFileInfo>
#include <QDir>
namespace Framework::Media
{
  QString animation_format_name(AnimationFormat format)
  {
    switch (format)
    {
      case AnimationFormat::Png: return "png";
      case AnimationFormat::Gif: return "gif";
      case AnimationFormat::Mp4: return "mp4";
    }
    return {};
  }
  bool valid_animation_export(const AnimationExportDefinition& value,QString& error)
  {
    error.clear();
    const auto format = animation_format_name(value.format);
    if (format.isEmpty() || value.width < 64 || value.height < 64 || value.width > 8192 || value.height > 8192 ||
        static_cast<qint64>(value.width)*value.height > 32000000 || value.frames == 0 || value.frames > 100000 ||
        value.fps < 1 || value.fps > 120 || (value.format == AnimationFormat::Gif && value.fps > 100))
    {
      error = "Invalid animation dimensions, frame count or FPS (GIF 1..100, other formats 1..120; at most 100000 frames).";
      return false;
    }
    if (value.format == AnimationFormat::Mp4 && (value.width%2 != 0 || value.height%2 != 0))
    {
      error = "MP4 width and height must be even for H.264/yuv420p.";
      return false;
    }
    const QFileInfo destination(value.path);
    if (value.path.trimmed().isEmpty() || destination.exists() || destination.isSymLink() || !destination.dir().exists() ||
        (value.format != AnimationFormat::Png && destination.suffix().compare(format,Qt::CaseInsensitive) != 0))
    {
      error = "Choose a new output path in an existing directory, with the selected format's extension.";
      return false;
    }
    if (!value.background.isValid() || value.background.alpha() != 255)
    {
      error = "The animation flattening background must be an opaque color.";
      return false;
    }
    return true;
  }
}
