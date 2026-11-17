#include "AnimationOutput.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
namespace Framework::Media
{
  AnimationOutput::AnimationOutput() = default;
  AnimationOutput::~AnimationOutput() = default;
  bool AnimationOutput::prepare(const AnimationExportDefinition& definition,QString& error)
  {
    if (!valid_animation_export(definition,error))
    {
      return false;
    }
    const auto parent = QFileInfo(definition.path).absoluteDir();
    temporary = std::make_unique<QTemporaryDir>(parent.filePath(".cubitsim-animation-XXXXXX"));
    if (!temporary->isValid() || !QDir().mkdir(frames_directory()))
    {
      error = "Cannot create animation staging directory beside the destination.";
      return false;
    }
    return true;
  }
  QString AnimationOutput::frames_directory() const
  {
    if (!temporary)
    {
      return {};
    }
    return QDir(temporary->path()).filePath("frames");
  }
  QString AnimationOutput::encoded_file(AnimationFormat format) const
  {
    if (!temporary)
    {
      return {};
    }
    return QDir(temporary->path()).filePath("animation."+animation_format_name(format));
  }
  bool AnimationOutput::publish(const AnimationExportDefinition& definition,QString& error)
  {
    bool accepted = false;
    const QFileInfo destination(definition.path);
    if (!destination.exists() && !destination.isSymLink())
    {
      if (definition.format == AnimationFormat::Png)
      {
        accepted = QDir().rename(frames_directory(),definition.path);
      }
      else
      {
        accepted = QFile::rename(encoded_file(definition.format),definition.path);
      }
    }
    if (!accepted)
    {
      error = "Cannot publish animation; the destination may already exist.";
    }
    return accepted;
  }
  bool AnimationOutput::cleanup()
  {
    return !temporary || temporary->remove();
  }
}
