#include "ReportSection.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportSection::ReportSection() = default;

QJsonObject ReportSection::to_json() const
{
  QJsonArray ts, ims, cs;
  for (const auto& item : tables)
  {
    ts.append(item.to_json());
  }
  for (const auto& item : images)
  {
    ims.append(item.to_json());
  }
  for (const auto& item : children)
  {
    cs.append(item.to_json());
  }
  return {{"id", QString::fromStdString(id)}, {"title", QString::fromStdString(title)},
    {"paragraphs", ReportJson::strings(paragraphs)}, {"tables", ts}, {"images", ims}, {"children", cs}};
}

bool ReportSection::from_json(const QJsonObject& input, ReportSection& output, std::string& error, int depth)
{
  ReportSection candidate;
  if (depth > 24 || !ReportJson::text(input, "id", candidate.id, error) ||
      !ReportJson::text(input, "title", candidate.title, error) ||
      !ReportJson::strings(input.value("paragraphs"), candidate.paragraphs, error) ||
      !input.value("tables").isArray() || !input.value("images").isArray() || !input.value("children").isArray())
  {
    error = "Invalid or excessively nested report section.";
    return false;
  }
  for (const auto value : input.value("tables").toArray())
  {
    ReportTable table;
    if (!value.isObject() || !ReportTable::from_json(value.toObject(), table, error))
    {
      return false;
    }
    candidate.tables.push_back(std::move(table));
  }
  for (const auto value : input.value("images").toArray())
  {
    ReportImage image;
    if (!value.isObject() || !ReportImage::from_json(value.toObject(), image, error))
    {
      return false;
    }
    candidate.images.push_back(std::move(image));
  }
  for (const auto value : input.value("children").toArray())
  {
    ReportSection child;
    if (!value.isObject() || !ReportSection::from_json(value.toObject(), child, error, depth + 1))
    {
      return false;
    }
    candidate.children.push_back(std::move(child));
  }
  output = std::move(candidate);
  return true;
}
