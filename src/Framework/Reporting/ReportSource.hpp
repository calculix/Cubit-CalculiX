#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>

class ReportSource
{
public:
  ReportSource();
  std::string workspace_id;
  std::string kind = "job";
  std::string model_id;
  int job_id = 0;
  std::vector<std::string> input_files;
  std::vector<std::string> fingerprints;
  std::string provenance;
  std::string run_identity;
  std::string snapshot_fingerprint;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportSource& output, std::string& error);
};
