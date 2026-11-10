#include "ReportSession.hpp"
#include <utility>
#include "Framework/Reporting/ReportJson.hpp"
#include <set>
#include <tuple>

ReportSession::ReportSession() = default;

QJsonObject ReportSession::to_json() const
{
  QJsonArray docs;
  for (const auto& item : documents)
  {
    docs.append(item.second->to_json());
  }
  QJsonObject draftsJson;
  for (const auto& entry : drafts)
  {
    draftsJson.insert(QString::fromStdString(entry.first), entry.second.to_json());
  }
  return {{"pending_request", pending_request.to_json()}, {"pending_source", pending_source}, {"drafts", draftsJson}, {"documents", docs}, {"active_id", QString::fromStdString(active_id)},
    {"request", request.to_json()}, {"configuration_changed", configuration_changed}};
}

bool ReportSession::from_json(const QJsonObject& input, ReportSession& output, std::string& error)
{
  ReportSession candidate;
  if (!input.value("documents").isArray() || !input.value("request").isObject() ||
      !input.value("configuration_changed").isBool() ||
      !ReportJson::text(input, "active_id", candidate.active_id, error) ||
      !ReportRequest::from_json(input.value("request").toObject(), candidate.request, error))
  {
    error = "Invalid saved Report session.";
    return false;
  }
  if (!input.value("pending_source").isBool() || !input.value("pending_request").isObject() ||
      !ReportRequest::from_json(input.value("pending_request").toObject(), candidate.pending_request, error))
  {
    error = "Invalid pending report source.";
    return false;
  }
  candidate.pending_source = input.value("pending_source").toBool();
  if (candidate.pending_source && (candidate.pending_request.provider.empty() ||
      (candidate.pending_request.kind == "job" && candidate.pending_request.job_id <= 0) ||
      (candidate.pending_request.kind == "model" && candidate.pending_request.model_id.empty())))
  {
    error = "Invalid pending report source identity.";
    return false;
  }
  const auto docs = input.value("documents").toArray();
  if (docs.size() > 1000)
  {
    error = "Saved report count exceeds the supported limit.";
    return false;
  }
  std::set<std::tuple<std::string, std::string, std::string, int>> sources;
  for (const auto value : docs)
  {
    auto document = std::make_shared<ReportDocument>();
    if (!value.isObject() || !ReportDocument::from_json(value.toObject(), *document, error) ||
        !candidate.documents.emplace(document->id, document).second)
    {
      return false;
    }
    const auto& source = document->source;
    const auto key = std::make_tuple(source.workspace_id, source.kind,
      source.kind == "model" ? source.model_id : std::string(), source.kind == "job" ? source.job_id : 0);
    if (!sources.insert(key).second)
    {
      error = "Saved report session contains more than one report for the same source.";
      return false;
    }
  }
  if ((!candidate.active_id.empty() && candidate.documents.count(candidate.active_id) == 0) ||
      (candidate.active_id.empty() && !candidate.documents.empty()))
  {
    error = "Invalid active report reference.";
    return false;
  }
  if (!candidate.active_id.empty())
  {
    const auto& active = *candidate.documents.at(candidate.active_id);
    if (candidate.request.provider != active.provider || candidate.request.kind != active.source.kind ||
        candidate.request.model_id != active.source.model_id || candidate.request.job_id != active.source.job_id)
    {
      error = "Saved report configuration does not match its active document source.";
      return false;
    }
  }
  if (!input.value("drafts").isObject())
  {
    error = "Missing report configuration drafts.";
    return false;
  }
  const auto draftsJson = input.value("drafts").toObject();
  for (auto it = draftsJson.begin(); it != draftsJson.end(); ++it)
  {
    ReportRequest draft;
    if (!it.value().isObject() || candidate.documents.count(it.key().toStdString()) == 0 ||
        !ReportRequest::from_json(it.value().toObject(), draft, error))
    {
      return false;
    }
    const auto& saved = *candidate.documents.at(it.key().toStdString());
    if (draft.provider != saved.provider || draft.kind != saved.source.kind ||
        draft.model_id != saved.source.model_id || draft.job_id != saved.source.job_id)
    {
      error = "Saved report draft does not match its document source.";
      return false;
    }
    candidate.drafts.emplace(it.key().toStdString(), std::move(draft));
  }
  candidate.configuration_changed = input.value("configuration_changed").toBool();
  output = std::move(candidate);
  return true;
}
