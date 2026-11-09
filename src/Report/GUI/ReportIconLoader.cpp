#include "ReportIconLoader.hpp"
#include <QDir>
#include <QFileInfo>

ReportIconLoader::ReportIconLoader() = default;

QIcon ReportIconLoader::load(const std::string& component_directory)
{
  const auto filename = QDir(QString::fromStdString(component_directory)).filePath("Icons/Report.svg");
  return QFileInfo::exists(filename) ? QIcon(filename) : QIcon();
}
