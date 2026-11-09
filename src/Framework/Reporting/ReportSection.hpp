#pragma once
#include "ReportTable.hpp"
#include "ReportImage.hpp"
#include <QJsonObject>
#include <string>
#include <vector>

class ReportSection
{
public:
  ReportSection();
  std::string id;
  std::string title;
  std::vector<std::string> paragraphs;
  std::vector<ReportTable> tables;
  std::vector<ReportImage> images;
  std::vector<ReportSection> children;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportSection& output, std::string& error, int depth = 0);
};
