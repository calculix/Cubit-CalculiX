#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>

class ReportImage
{
public:
  ReportImage();
  std::string id;
  std::string caption;
  std::string media_type;
  std::string base64_data;
  QJsonObject provenance;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportImage& output, std::string& error);
};
