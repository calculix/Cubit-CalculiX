#include "CaptureDefinition.hpp"
#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>
namespace Postprocessor
{
  bool write_capture_png(const std::string& path,const QByteArray& png,std::string& error)
  {
    error.clear();
    const QFileInfo target(QString::fromUtf8(path.c_str()));
    if (path.empty() || target.suffix().compare("png",Qt::CaseInsensitive) != 0 || target.exists() || target.isSymLink() ||
        !target.dir().exists() || png.isEmpty() || png.size() > CaptureDefaults::maximum_png_bytes)
    {
      error = "Choose a new .png filename in an existing directory. Captures cannot overwrite files or exceed 32 MiB.";
      return false;
    }
    QTemporaryFile temporary(target.dir().filePath(".cubitsim-capture-XXXXXX.tmp"));
    if (!temporary.open() || temporary.write(png) != png.size() || !temporary.flush())
    {
      error = "Cannot write complete capture PNG.";
      return false;
    }
    temporary.close();
    if (!QFile::rename(temporary.fileName(),target.absoluteFilePath()))
    {
      error = "Cannot publish capture; the destination may already exist.";
      return false;
    }
    temporary.setAutoRemove(false);
    return true;
  }
}
