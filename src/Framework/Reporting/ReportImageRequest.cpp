#include "ReportImageRequest.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportImageRequest::ReportImageRequest() = default;

QJsonObject ReportImageRequest::to_json() const
{
  QJsonObject result;
  result.insert("provider", QString::fromStdString(provider));
  result.insert("purpose", QString::fromStdString(purpose));
  result.insert("context", QString::fromStdString(context));
  result.insert("source", source.to_json());
  result.insert("settings", settings.to_json());
  result.insert("width", width);
  result.insert("height", height);
  if (visual)
  {
    result.insert("visual",QJsonObject{{"kind",visual->kind == ReportVisualKind::View ? "view" : "plot"},
      {"reference",QString::fromStdString(visual->reference)}});
  }
  return result;
}

bool ReportImageRequest::from_json(const QJsonObject& input, ReportImageRequest& output, std::string& error)
{
  ReportImageRequest candidate;
  if (!ReportJson::text(input, "purpose", candidate.purpose, error) ||
      !ReportJson::text(input, "context", candidate.context, error))
  {
    return false;
  }
  if (!(ReportJson::text(input, "provider", candidate.provider, error)))
  {
    return false;
  }
  if (!(input.value("source").isObject() && ReportSource::from_json(input.value("source").toObject(), candidate.source, error)))
  {
    return false;
  }
  if (!(input.value("settings").isObject() && ReportConfiguration::from_json(input.value("settings").toObject(), candidate.settings, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "width", candidate.width, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "height", candidate.height, error)))
  {
    return false;
  }
  if (input.contains("visual"))
  {
    const auto value = input.value("visual");
    const auto object = value.toObject();
    const auto kind = object.value("kind").toString();
    if (!value.isObject() || (kind != "view" && kind != "plot") ||
        !object.value("reference").isString() || object.value("reference").toString().isEmpty())
    {
      error = "Invalid report visual reference.";
      return false;
    }
    candidate.visual = ReportVisualReference{kind == "view" ? ReportVisualKind::View : ReportVisualKind::Plot,
      object.value("reference").toString().toStdString()};
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
