#include "PostprocessorIconLoader.hpp"

#include <QDir>
#include <QFileInfo>

QIcon PostprocessorIconLoader::load(const std::string& directory, const std::string& name)
{
  const QString filepath = QDir(QString::fromUtf8(directory.c_str())).filePath(QString::fromStdString(name) + ".svg");

  if (QFileInfo::exists(filepath))
  {
    return QIcon(filepath);
  }

  return QIcon();
}
