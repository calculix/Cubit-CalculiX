#include "PostprocessorReportResultImages.hpp"
#include "PostprocessorReportBlockSummary.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include "PostprocessorReportCapturePlan.hpp"
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
  const auto diagnose = [&](const std::string& message, const std::string& severity = "unavailable")
  {
    ReportDiagnostic diagnostic;
    diagnostic.section_id = "post.images";
    diagnostic.severity = severity;
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
  PostprocessorReportCapturePlan capturePlan;
  if (!summaries.empty())
  {
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
          "Freezing Postprocessor presentation", 0, summaries.size(), error))
    {
      return false;
    }
    Postprocessor::CaptureDefinition definition;
    if (request.visual)
    {
      if (request.visual->kind != ReportVisualKind::View)
      {
        error = "Block result images require a Postprocessor view.";
        return false;
      }
      definition.name = request.visual->reference;
    }
    else
    {
      const auto view = request.settings.values.find("post.images.view");
      if (view != request.settings.values.end())
      {
        definition.name = view->second;
      }
    }
    if (request.width != 0 || request.height != 0)
    {
      definition.width = request.width;
      definition.height = request.height;
    }
    Postprocessor::CaptureSnapshot snapshot;
    std::string captureSource;
    std::string captureError;
    if (application.capture_snapshot(definition, snapshot, captureError))
    {
      for (const auto& item : snapshot.sources)
      {
        if (item.second == source)
        {
          captureSource = item.first;
          break;
        }
      }
    }
    if (!captureError.empty() || !capturePlan.initialize(std::move(snapshot), captureSource,
          request.settings, captureError))
    {
      diagnose("Result image presentation unavailable: " + captureError);
      output = std::move(candidate);
      error.clear();
      return true;
    }
  }
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
      Postprocessor::CaptureSnapshot snapshot;
      if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
            "Preparing block filters and vectors / " + summary.component,
            completed, summaries.size(), error))
      {
        return false;
      }
      if (!capturePlan.prepare(frame, summary.block.id, selection, snapshot, captureError))
      {
        diagnose(section.title + ": " + captureError);
        continue;
      }
      if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
            "Rendering block image / " + summary.component, completed, summaries.size(), error))
      {
        return false;
      }
      if (!application.render_report_result(snapshot, image, captureError))
      {
        diagnose(section.title + ": " + captureError);
        continue;
      }
      if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
            "Storing block image / " + summary.component, completed, summaries.size(), error))
      {
        return false;
      }
      image.id = summary.id + ".frame." + std::to_string(frame);
      image.caption = section.title + " / step " + std::to_string(source->increments[frame].step) +
        " / increment " + std::to_string(source->increments[frame].increment);
      const auto capture = image.provenance.value("report_capture").toObject();
      const auto warning = image.provenance.value("presentation_warning").toString();
      if (!warning.isEmpty())
      {
        diagnose(section.title + ": " + warning.toStdString(), "warning");
        image.caption += " / presentation warning (see diagnostics)";
      }
      if (capture.value("filtered").toBool())
      {
        image.caption += " / inherited filters; extrema describe the complete original block";
      }
      if (image.provenance.value("presentation_fallback").toBool())
      {
        image.caption += " / default presentation (no matching visible representation)";
      }
      if (capture.value("automatic_range_adjustment").toBool())
      {
        image.caption += " / automatic range (inherited limits have a different or unresolved quantity)";
      }
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
