#pragma once
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportRequest.hpp"
#include <memory>
#include <map>

class ReportSession
{
public:
  ReportSession();
  std::map<std::string, std::shared_ptr<const ReportDocument>> documents;
  std::string active_id;
  ReportRequest request;
  ReportRequest pending_request;
  bool pending_source = false;
  std::map<std::string, ReportRequest> drafts;
  bool configuration_changed = false;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportSession& output, std::string& error);
};
