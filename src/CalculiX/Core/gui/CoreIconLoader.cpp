#include "Core/gui/CoreIconLoader.hpp"

#include <QFileInfo>

QIcon CoreIconLoader::load(const QString& directory, const std::string& name)
{
  const QString filepath = directory + QString::fromStdString(name) + ".svg";

  if (QFileInfo::exists(filepath))
  {
    return QIcon(filepath);
  }

  return QIcon();
}
