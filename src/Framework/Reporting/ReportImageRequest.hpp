#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>
#include "ReportSource.hpp"
#include "ReportConfiguration.hpp"

class ReportImageRequest
{
public:
  ReportImageRequest();
  std::string provider;
  ReportSource source;
  ReportConfiguration settings;
  int width = 0;
  int height = 0;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportImageRequest& output, std::string& error);
};
