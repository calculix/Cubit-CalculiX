#include "ReportDocument.hpp"
#include <utility>
#include "ReportJson.hpp"
#include <QByteArray>
#include <set>
#include <functional>

ReportDocument::ReportDocument() = default;

QJsonObject ReportDocument::to_json() const
{
  QJsonArray content, issues;
  for (const auto& item : sections)
  {
    content.append(item.to_json());
  }
  for (const auto& item : diagnostics)
  {
    issues.append(item.to_json());
  }
  return {{"id", QString::fromStdString(id)}, {"title", QString::fromStdString(title)},
    {"provider", QString::fromStdString(provider)}, {"template_id", QString::fromStdString(template_id)}, {"template_revision", template_revision},
    {"profile_id", QString::fromStdString(profile_id)}, {"profile_revision", profile_revision},
    {"generated_at", QString::fromStdString(generated_at)}, {"source", source.to_json()},
    {"configuration", configuration.to_json()}, {"sections", content}, {"diagnostics", issues},
    {"notes", ReportJson::strings(notes)}};
}

bool ReportDocument::validate(std::string& error) const
{
  if (id.empty() || title.empty() || provider.empty() || template_id.empty() || template_revision < 1 ||
      source.workspace_id.empty() ||
      (source.kind != "job" && source.kind != "model") ||
      (source.kind == "job" && source.job_id <= 0) ||
      (source.kind == "model" && (source.model_id.empty() || source.snapshot_fingerprint.empty())) ||
      source.input_files.size() != source.fingerprints.size())
  {
    error = "Report document identity or provenance is invalid.";
    return false;
  }
  std::set<std::string> ids;
  std::function<bool(const ReportSection&, int)> check = [&](const ReportSection& section, int depth)
  {
    if (depth > 24 || section.id.empty() || section.title.empty() || !ids.insert(section.id).second)
    {
      error = "Report sections require unique stable IDs and titles.";
      return false;
    }
    for (const auto& table : section.tables)
    {
      if (!table.validate(error))
      {
        return false;
      }
    }
    for (const auto& image : section.images)
    {
      if (image.id.empty() || image.base64_data.empty() ||
          (image.media_type != "image/png" && image.media_type != "image/jpeg") ||
          QByteArray::fromBase64(QByteArray::fromStdString(image.base64_data)).isEmpty())
      {
        error = "Report contains an invalid embedded image.";
        return false;
      }
    }
    for (const auto& child : section.children)
    {
      if (!check(child, depth + 1))
      {
        return false;
      }
    }
    return true;
  };
  for (const auto& section : sections)
  {
    if (!check(section, 0))
    {
      return false;
    }
  }
  error.clear();
  return true;
}

bool ReportDocument::from_json(const QJsonObject& input, ReportDocument& output, std::string& error)
{
  ReportDocument candidate;
  if (!ReportJson::text(input, "id", candidate.id, error) || !ReportJson::text(input, "title", candidate.title, error) ||
      !ReportJson::text(input, "provider", candidate.provider, error) ||
      !ReportJson::text(input, "template_id", candidate.template_id, error) ||
      !ReportJson::integer(input, "template_revision", candidate.template_revision, error) ||
      !ReportJson::text(input, "profile_id", candidate.profile_id, error) ||
      !ReportJson::integer(input, "profile_revision", candidate.profile_revision, error) ||
      !ReportJson::text(input, "generated_at", candidate.generated_at, error) ||
      !input.value("source").isObject() || !ReportSource::from_json(input.value("source").toObject(), candidate.source, error) ||
      !input.value("configuration").isObject() || !ReportConfiguration::from_json(input.value("configuration").toObject(), candidate.configuration, error) ||
      !ReportJson::strings(input.value("notes"), candidate.notes, error) ||
      !input.value("sections").isArray() || !input.value("diagnostics").isArray())
  {
    return false;
  }
  for (const auto value : input.value("sections").toArray())
  {
    ReportSection section;
    if (!value.isObject() || !ReportSection::from_json(value.toObject(), section, error))
    {
      return false;
    }
    candidate.sections.push_back(std::move(section));
  }
  for (const auto value : input.value("diagnostics").toArray())
  {
    ReportDiagnostic issue;
    if (!value.isObject() || !ReportDiagnostic::from_json(value.toObject(), issue, error))
    {
      return false;
    }
    candidate.diagnostics.push_back(std::move(issue));
  }
  if (!candidate.validate(error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}
