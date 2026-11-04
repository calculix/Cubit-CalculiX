#include "Core/gui/CoreIconLoader.hpp"

#include <QDir>
#include <QFileInfo>

QIcon CoreIconLoader::load(const QString& directory, const std::string& name)
{
  const QString filepath = QDir(directory).filePath(QString::fromStdString(name) + ".svg");

  if (QFileInfo::exists(filepath))
  {
    return QIcon(filepath);
  }

  return QIcon();
}
