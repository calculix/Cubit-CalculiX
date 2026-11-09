#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>
#include "ReportConfiguration.hpp"

class ReportRequest
{
public:
  ReportRequest();
  std::string provider;
  std::string kind = "job";
  std::string model_id;
  int job_id = 0;
  std::string profile_id;
  int profile_revision = 0;
  ReportConfiguration overrides;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportRequest& output, std::string& error);
};
