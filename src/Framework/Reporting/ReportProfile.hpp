#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>
#include "ReportConfiguration.hpp"

class ReportProfile
{
public:
  ReportProfile();
  std::string id;
  std::string name;
  std::string workspace_id;
  int revision = 0;
  std::string template_id;
  int template_revision = 0;
  ReportConfiguration configuration;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportProfile& output, std::string& error);
};
