#include "PostprocessorReportFrameSelection.hpp"
#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <map>
#include <set>
#include <utility>

PostprocessorReportFrameSelection::PostprocessorReportFrameSelection() = default;

bool PostprocessorReportFrameSelection::envelope(const ReportConfiguration& settings)
{
  const auto found = settings.values.find("post.results.frames");
  return found != settings.values.end() && found->second == "envelope";
}

bool PostprocessorReportFrameSelection::select(const SimResults::VtkResultsData& source,
  const ReportConfiguration& settings, std::vector<std::size_t>& output, std::string& error) const
{
  const auto choice = settings.values.find("post.results.frames");
  if (choice == settings.values.end() || (choice->second != "last_per_step" &&
      choice->second != "envelope" && choice->second != "all_frames"))
  {
    error = "Report frame selection must be resolved from the workspace template.";
    return false;
  }
  std::set<int> steps;
  const auto selection = settings.values.find("steps");
  if (selection != settings.values.end() && selection->second != "all")
  {
    const auto text = QString::fromStdString(selection->second);
    if (!QRegularExpression("^[1-9][0-9]*(,[1-9][0-9]*)*$").match(text).hasMatch())
    {
      error = "Report steps must be all or comma-separated positive ordinals.";
      return false;
    }
    for (const auto& item : text.split(','))
    {
      bool ok = false;
      const int step = item.toInt(&ok);
      if (!ok)
      {
        error = "Report step ordinal is outside the supported range.";
        return false;
      }
      steps.insert(step);
    }
  }
  std::vector<std::size_t> selected;
  std::map<int, std::size_t> last;
  for (std::size_t index = 0; index < source.increments.size(); ++index)
  {
    const auto& frame = source.increments[index];
    if (frame.step <= 0)
    {
      error = "Result frame has no verified positive solver step ordinal.";
      return false;
    }
    if (!steps.empty() && steps.count(frame.step) == 0)
    {
      continue;
    }
    selected.push_back(index);
    last[frame.step] = index;
  }
  if (choice->second == "last_per_step")
  {
    std::vector<std::size_t> candidate;
    for (auto index : selected)
    {
      if (last.at(source.increments[index].step) == index)
      {
        candidate.push_back(index);
      }
    }
    selected = std::move(candidate);
  }
  output = std::move(selected);
  error.clear();
  return true;
}
