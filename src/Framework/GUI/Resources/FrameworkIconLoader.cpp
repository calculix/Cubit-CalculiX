#include "FrameworkIconLoader.hpp"
#include "CubitMessage.hpp"
#include <QDir>
#include <QFileInfo>

QIcon FrameworkIconLoader::load(const std::string& componentDirectory, const std::string& filename) {
  const QString path = QDir(QString::fromStdString(componentDirectory)).filePath("Icons/" + QString::fromStdString(filename));
  if (!QFileInfo(path).isReadable()) {
    PRINT_ERROR("Cannot load Framework icon: %s\n", path.toUtf8().constData());
    return QIcon();
  }
  return QIcon(path);
}
