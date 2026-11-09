#include "ReportSource.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportSource::ReportSource() = default;

QJsonObject ReportSource::to_json() const
{
  QJsonObject result;
  result.insert("snapshot_fingerprint", QString::fromStdString(snapshot_fingerprint));
  result.insert("kind", QString::fromStdString(kind));
  result.insert("model_id", QString::fromStdString(model_id));
  result.insert("workspace_id", QString::fromStdString(workspace_id));
  result.insert("job_id", job_id);
  result.insert("input_files", ReportJson::strings(input_files));
  result.insert("fingerprints", ReportJson::strings(fingerprints));
  result.insert("provenance", QString::fromStdString(provenance));
  result.insert("run_identity", QString::fromStdString(run_identity));
  return result;
}

bool ReportSource::from_json(const QJsonObject& input, ReportSource& output, std::string& error)
{
  ReportSource candidate;
  if (!ReportJson::text(input, "kind", candidate.kind, error) ||
      !ReportJson::text(input, "model_id", candidate.model_id, error) ||
      (candidate.kind != "job" && candidate.kind != "model"))
  {
    error = "Invalid report source kind.";
    return false;
  }
  if (!(ReportJson::text(input, "workspace_id", candidate.workspace_id, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "job_id", candidate.job_id, error)))
  {
    return false;
  }
  if (!(ReportJson::strings(input.value("input_files"), candidate.input_files, error)))
  {
    return false;
  }
  if (!(ReportJson::strings(input.value("fingerprints"), candidate.fingerprints, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "provenance", candidate.provenance, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "run_identity", candidate.run_identity, error)))
  {
    return false;
  }
  if (!ReportJson::text(input, "snapshot_fingerprint", candidate.snapshot_fingerprint, error))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
