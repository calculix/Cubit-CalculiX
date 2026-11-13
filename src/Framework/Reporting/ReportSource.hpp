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
  // Durable solver launch/setup IDs, independent of result-conversion tokens.
  std::string solver_launch_id;
  std::string setup_snapshot_id;
  std::string snapshot_fingerprint;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportSource& output, std::string& error);
};
