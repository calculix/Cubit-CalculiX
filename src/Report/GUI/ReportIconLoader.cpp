#include "ReportIconLoader.hpp"
#include <QDir>
#include <QFileInfo>

ReportIconLoader::ReportIconLoader() = default;

QIcon ReportIconLoader::load(const std::string& component_directory, const std::string& filename)
{
  const auto path = QDir(QString::fromStdString(component_directory)).filePath("Icons/" + QString::fromStdString(filename));
  return QFileInfo::exists(path) ? QIcon(path) : QIcon();
}
