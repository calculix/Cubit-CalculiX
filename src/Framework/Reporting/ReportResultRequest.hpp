#pragma once
#include <QJsonObject>
#include "ReportProgress.hpp"
#include <map>
#include <string>
#include <vector>
#include "ReportSource.hpp"
#include "ReportConfiguration.hpp"

class ReportResultRequest
{
public:
  ReportResultRequest();
  // Borrowed for synchronous reporting only; never serialized.
  ReportProgress* progress = nullptr;
  std::string provider;
  ReportSource source;
  ReportConfiguration settings;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportResultRequest& output, std::string& error);
};
