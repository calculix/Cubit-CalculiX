#pragma once
#include <QIcon>
#include <string>

class ReportIconLoader
{
public:
  ReportIconLoader();
  static QIcon load(const std::string& component_directory);
};
