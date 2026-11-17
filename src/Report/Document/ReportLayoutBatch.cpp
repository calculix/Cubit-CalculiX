#include "ReportLayoutBatch.hpp"
#include "ReportDocumentLayout.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include <QString>
#include <algorithm>
#include <functional>
#include <set>
#include <utility>

ReportLayoutBatch::ReportLayoutBatch() = default;
ReportLayoutBatch::~ReportLayoutBatch() = default;

bool ReportLayoutBatch::plan(const ReportDocument& document,
  std::vector<std::unique_ptr<ReportLayoutBatch>>& output, ReportProgress* progress, std::string& error)
{
  const auto rowsLimit = static_cast<std::size_t>(ReportUserOptions::preview_batch_rows());
  const auto textLimit = static_cast<std::size_t>(ReportUserOptions::preview_batch_characters());
  const auto cellsLimit = static_cast<std::size_t>(ReportUserOptions::preview_batch_cells());
  std::vector<std::unique_ptr<ReportLayoutBatch>> candidate;
  const auto blank = [&]()
  {
    auto result = std::make_unique<ReportLayoutBatch>();
    auto& fragment = result->fragment;
    fragment.id = document.id;
    fragment.title = document.title;
    fragment.provider = document.provider;
    fragment.template_id = document.template_id;
    fragment.template_revision = document.template_revision;
    fragment.profile_id = document.profile_id;
    fragment.profile_revision = document.profile_revision;
    fragment.generated_at = document.generated_at;
    fragment.source = document.source;
    fragment.configuration = document.configuration;
    fragment.configuration.values["cover"] = "false";
    fragment.configuration.values["contents"] = "false";
    return result;
  };
  const auto setting = [&](const std::string& key)
  {
    const auto found = document.configuration.values.find(key);
    return found == document.configuration.values.end() || found->second == "true";
  };
  if (setting("cover"))
  {
    auto cover = blank();
    cover->fragment.configuration.values["cover"] = "true";
    candidate.push_back(std::move(cover));
  }
  if (setting("contents"))
  {
    auto contents = blank();
    contents->contents_only = true;
    contents->fragment.configuration.values["contents"] = "true";
    for (const auto& section : document.sections)
    {
      if (contents->fragment.sections.size() >= rowsLimit)
      {
        candidate.push_back(std::move(contents));
        contents = blank();
        contents->contents_only = true;
        contents->fragment.configuration.values["contents"] = "true";
      }
      ReportSection heading;
      heading.id = section.id;
      heading.title = section.title;
      contents->fragment.sections.push_back(std::move(heading));
    }
    candidate.push_back(std::move(contents));
  }
  auto pending = blank();
  std::size_t rows = 0, characters = 0, cells = 0;
  const auto flush = [&]()
  {
    if (!pending->fragment.sections.empty())
    {
      candidate.push_back(std::move(pending));
      pending = blank();
    }
    rows = 0;
    characters = 0;
    cells = 0;
  };
  const auto append = [&](ReportSection part, std::size_t partRows, std::size_t partText, std::size_t partCells)
  {
    if (rows + partRows > rowsLimit || characters + partText > textLimit || cells + partCells > cellsLimit)
    {
      flush();
    }
    rows += partRows;
    characters += partText;
    cells += partCells;
    if (!pending->fragment.sections.empty() && pending->fragment.sections.back().id == part.id)
    {
      auto& target = pending->fragment.sections.back();
      target.paragraphs.insert(target.paragraphs.end(), part.paragraphs.begin(), part.paragraphs.end());
      for (auto& table : part.tables)
      {
        target.tables.push_back(std::move(table));
      }
      for (auto& image : part.images)
      {
        target.images.push_back(std::move(image));
      }
    }
    else
    {
      pending->fragment.sections.push_back(std::move(part));
    }
  };
  const auto heading = [](const ReportSection& source)
  {
    ReportSection result;
    result.id = source.id;
    result.title = source.title;
    return result;
  };
  std::set<std::string> notes;
  std::function<bool(const ReportSection&)> section = [&](const ReportSection& source)
  {
    if (!ReportProgress::advance(progress, ReportProgressStage::PreviewContent,
          "Planning preview batches", 0, 0, error))
    {
      return false;
    }
    if (source.title.size() > textLimit)
    {
      error = "Report heading exceeds the bounded preview batch size.";
      return false;
    }
    append(heading(source), 0, source.title.size(), 0);
    for (const auto& line : source.paragraphs)
    {
      const auto text = QString::fromStdString(line);
      for (qsizetype offset = 0; offset < text.size(); offset += static_cast<qsizetype>(textLimit))
      {
        auto part = heading(source);
        const auto chunk = text.mid(offset, static_cast<qsizetype>(textLimit));
        part.paragraphs.push_back(chunk.toStdString());
        append(std::move(part), 0, static_cast<std::size_t>(chunk.size()), 0);
      }
    }
    for (const auto& table : source.tables)
    {
      std::size_t tableCharacters = table.title.size();
      for (const auto& column : table.columns)
      {
        tableCharacters += column.size();
      }
      for (const auto& unit : table.units)
      {
        tableCharacters += unit.size() + 3;
      }
      if (table.columns.size() > cellsLimit || tableCharacters > textLimit)
      {
        error = "Report table header exceeds the bounded preview batch size.";
        return false;
      }
      const auto blankTable = [&]()
      {
        ReportTable result;
        result.title = table.title;
        result.columns = table.columns;
        result.units = table.units;
        return result;
      };
      ReportTable fragment = blankTable();
      std::size_t textCount = tableCharacters;
      const auto tableFlush = [&]()
      {
        if (fragment.rows.empty())
        {
          return;
        }
        auto part = heading(source);
        const auto count = fragment.rows.size();
        part.tables.push_back(std::move(fragment));
        append(std::move(part), count + 1, textCount, (count + 1) * table.columns.size());
        fragment = blankTable();
        fragment.title += " (continued)";
        textCount = tableCharacters;
      };
      if (table.rows.empty())
      {
        auto part = heading(source);
        part.tables.push_back(fragment);
        append(std::move(part), 1, textCount, table.columns.size());
      }
      for (const auto& row : table.rows)
      {
        // Split exceptionally large textual cells across continuation rows.
        // The original semantic table remains intact for ODT and persistence.
        const auto perCell = (std::max)(std::size_t(1),
          (textLimit - tableCharacters) / (std::max)(std::size_t(1), table.columns.size()));
        std::size_t segments = 1;
        for (const auto& value : row)
        {
          if (value.isString())
          {
            segments = (std::max)(segments,
              (static_cast<std::size_t>(value.toString().size()) + perCell - 1) / perCell);
          }
        }
        for (std::size_t segment = 0; segment < segments; ++segment)
        {
          std::vector<QJsonValue> values;
          std::size_t valueText = 0;
          for (const auto& value : row)
          {
            if (value.isString())
            {
              const auto text = value.toString().mid(static_cast<qsizetype>(segment * perCell), static_cast<qsizetype>(perCell));
              valueText += static_cast<std::size_t>(text.size());
              values.push_back(text);
            }
            else if (segment == 0)
            {
              valueText += 32;
              values.push_back(value);
            }
            else
            {
              values.push_back("");
            }
          }
          if (valueText + tableCharacters > textLimit || table.columns.size() * 2 > cellsLimit)
          {
            error = "Report row exceeds the bounded preview batch size.";
            return false;
          }
          if (fragment.rows.size() + 2 > rowsLimit || textCount + valueText > textLimit ||
              (fragment.rows.size() + 2) * table.columns.size() > cellsLimit)
          {
            tableFlush();
          }
          fragment.rows.push_back(std::move(values));
          textCount += valueText;
        }
        if (fragment.rows.size() % 32 == 0 && !ReportProgress::advance(progress,
              ReportProgressStage::PreviewContent, "Planning preview batches", 0, 0, error))
        {
          return false;
        }
      }
      tableFlush();
    }
    for (const auto& image : source.images)
    {
      flush();
      auto part = heading(source);
      part.images.push_back(image);
      append(std::move(part), 0, image.caption.size(), 0);
      flush();
    }
    const auto note = document.notes.find(source.id);
    if (note != document.notes.end())
    {
      notes.insert(note->first);
      ReportSection part = heading(source);
      part.paragraphs.push_back("Notes: " + note->second);
      // Reuse paragraph splitting without recursively collecting the same note.
      const auto text = QString::fromStdString(part.paragraphs.front());
      for (qsizetype offset = 0; offset < text.size(); offset += static_cast<qsizetype>(textLimit))
      {
        auto chunk = heading(source);
        chunk.paragraphs.push_back(text.mid(offset, static_cast<qsizetype>(textLimit)).toStdString());
        append(std::move(chunk), 0, (std::min)(textLimit, static_cast<std::size_t>(text.size() - offset)), 0);
      }
    }
    for (const auto& child : source.children)
    {
      if (!section(child))
      {
        return false;
      }
    }
    return true;
  };
  for (const auto& item : document.sections)
  {
    if (!section(item))
    {
      return false;
    }
  }
  for (const auto& note : document.notes)
  {
    if (notes.count(note.first) == 0 && !note.second.empty())
    {
      ReportSection retained;
      retained.id = "report.note." + note.first;
      retained.title = "Retained note / " + note.first;
      retained.paragraphs.push_back(note.second);
      if (!section(retained))
      {
        return false;
      }
    }
  }
  ReportSection diagnostics;
  diagnostics.id = "report.diagnostics";
  diagnostics.title = "Sources and diagnostics";
  diagnostics.paragraphs.push_back(document.source.provenance);
  if (!document.source.snapshot_fingerprint.empty())
  {
    diagnostics.paragraphs.push_back("Reporting snapshot SHA-256: " + document.source.snapshot_fingerprint);
  }
  for (std::size_t index = 0; index < document.source.input_files.size(); ++index)
  {
    diagnostics.paragraphs.push_back(document.source.input_files[index]);
    diagnostics.paragraphs.push_back("SHA-256: " + document.source.fingerprints[index]);
  }
  for (const auto& issue : document.diagnostics)
  {
    diagnostics.paragraphs.push_back(issue.severity + " / " + issue.section_id + ": " + issue.message);
  }
  if (!section(diagnostics))
  {
    return false;
  }
  flush();
  output = std::move(candidate);
  error.clear();
  return true;
}

bool ReportLayoutBatch::prepare(std::string& error)
{
  auto candidate = std::make_unique<ReportDocumentLayout>();
  if (!candidate->prepare_single(fragment, error, nullptr, contents_only))
  {
    return false;
  }
  prepared = std::move(candidate);
  fragment.sections.clear();
  error.clear();
  return true;
}

bool ReportLayoutBatch::resolve_contents(const std::vector<std::pair<std::string, int>>& pages, std::string& error)
{
  return prepared->resolve_contents(pages, error);
}

const ReportDocumentLayout& ReportLayoutBatch::layout() const
{
  return *prepared;
}
