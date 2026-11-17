#include "PostprocessorReportResultImages.hpp"
#include "PostprocessorReportBlockSummary.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include <set>
#include <utility>

PostprocessorReportResultImages::PostprocessorReportResultImages() = default;

bool PostprocessorReportResultImages::collect(const PostprocessorApplicationInterface& application,
  const ReportImageRequest& request, ReportDocument& output, std::string& error) const
{
  if (request.source.kind != "job")
  {
    error.clear();
    return true;
  }
  std::shared_ptr<const SimResults::VtkResultsData> source;
  if (!PostprocessorReportBlockSummary::source(application, request.source, source, error))
  {
    return false;
  }
  ReportDocument candidate = output;
  const auto diagnose = [&](const std::string& message)
  {
    ReportDiagnostic diagnostic;
    diagnostic.section_id = "post.images";
    diagnostic.severity = "unavailable";
    diagnostic.message = message;
    candidate.diagnostics.push_back(std::move(diagnostic));
  };
  if (!source)
  {
    diagnose("Open results for this exact solver launch in Postprocessor to include result images.");
    output = std::move(candidate);
    error.clear();
    return true;
  }
  ReportResultRequest summaryRequest;
  summaryRequest.progress = request.progress;
  summaryRequest.provider = request.provider;
  summaryRequest.source = request.source;
  summaryRequest.settings = request.settings;
  std::vector<PostprocessorReportBlockSummary> summaries;
  if (!PostprocessorReportBlockSummary::collect(summaryRequest, *source, summaries, error, &candidate.diagnostics))
  {
    return false;
  }
  ReportSection images;
  images.id = "post.images";
  images.title = "Block component result images";
  std::size_t completed = 0;
  for (const auto& summary : summaries)
  {
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
          "Rendering block " + std::to_string(summary.block.model_id) + " / " + summary.component,
          completed, summaries.size(), error))
    {
      return false;
    }
    if (!summary.available)
    {
      ++completed;
      continue;
    }
    auto section = summary.section(false);
    section.id += ".images";
    std::set<std::size_t> frames = {summary.minimum_frame, summary.maximum_frame};
    for (auto frame : frames)
    {
      Postprocessor::FieldSelection selection;
      selection.dataset_id = summary.block.id;
      selection.array_name = summary.field.array_name;
      selection.association = summary.field.association;
      selection.component = summary.component;
      selection.group_key = Postprocessor::field_group_key(summary.field);
      ReportImage image;
      std::string captureError;
      if (!application.render_report_result(request.source, frame, summary.block.id,
            selection, image, captureError))
      {
        diagnose(section.title + ": " + captureError);
        continue;
      }
      image.id = summary.id + ".frame." + std::to_string(frame);
      image.caption = section.title + " / step " + std::to_string(source->increments[frame].step) +
        " / increment " + std::to_string(source->increments[frame].increment);
      if (frames.size() > 1)
      {
        if (frame == summary.minimum_frame)
        {
          image.caption += " / minimum governing frame";
        }
        else
        {
          image.caption += " / maximum governing frame";
        }
      }
      section.images.push_back(std::move(image));
    }
    images.children.push_back(std::move(section));
    ++completed;
  }
  if (summaries.empty())
  {
    diagnose("No verified model blocks with result components exist in the selected frames.");
  }
  if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
        "Block result images complete", completed, summaries.size(), error))
  {
    return false;
  }
  candidate.sections.push_back(std::move(images));
  output = std::move(candidate);
  error.clear();
  return true;
}
