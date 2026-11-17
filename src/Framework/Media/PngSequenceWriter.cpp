#include "PngSequenceWriter.hpp"
#include <QSaveFile>
#include <QDir>
#include <QJsonDocument>
namespace Framework::Media
{
  QString PngSequenceWriter::frame_path(const QString& directory,std::size_t index)
  {
    return QDir(directory).filePath(QString("frame_%1.png").arg(static_cast<qulonglong>(index),6,10,QChar('0')));
  }
  bool PngSequenceWriter::write_frame(const QString& directory,std::size_t index,const QImage& image,QString& error)
  {
    QSaveFile file(frame_path(directory,index));
    file.setDirectWriteFallback(false);
    if (image.isNull() || !file.open(QIODevice::WriteOnly) || !image.save(&file,"PNG") || !file.commit())
    {
      error = "Cannot write animation PNG frame.";
      return false;
    }
    return true;
  }
  bool PngSequenceWriter::write_manifest(const QString& directory,const AnimationExportDefinition& definition,QString& error)
  {
    auto object = definition.metadata;
    object.insert("width",definition.width);
    object.insert("height",definition.height);
    object.insert("fps",definition.fps);
    object.insert("format","png");
    object.insert("loop",definition.loop);
    object.insert("frames",static_cast<int>(definition.frames));
    object.insert("duration_seconds",static_cast<double>(definition.frames)/definition.fps);
    object.insert("pattern","frame_%06d.png");
    object.insert("first_frame",0);
    QSaveFile file(QDir(directory).filePath("animation.json"));
    file.setDirectWriteFallback(false);
    const auto json = QJsonDocument(object).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit())
    {
      error = "Cannot write animation sequence manifest.";
      return false;
    }
    return true;
  }
}
