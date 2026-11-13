#include "CalculiXReportImageProvider.hpp"
#include "CalculiXReportImageSettings.hpp"
#include "Core/drawing/CalculiXCaptureInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/jobs/CalculiXJobRunRecord.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include <QBuffer>
#include <QImageReader>
#include <QSize>
#include <algorithm>
#include <iterator>
#include <utility>

namespace
{
bool currentModel(const ReportSource& source)
{
  return source.workspace_id == "calculix" && source.kind == "model" && source.model_id == "current" && !source.snapshot_fingerprint.empty();
}

bool archivedSetup(const ReportSource& source, std::shared_ptr<const CalculiXJobRunRecord>& output, std::string& error)
{
  if (source.kind != "job" || source.solver_launch_id.empty() ||
      !CalculiXJobsInterface().get_run_record(source.job_id, output, error))
  {
    if (error.empty())
    {
      error = "The job has no verified archived setup.";
    }
    return false;
  }
  if (output->launchId != source.solver_launch_id || !output->setup || output->setup->id != source.setup_snapshot_id ||
      output->setup->fingerprint != source.snapshot_fingerprint)
  {
    error = output->setupUnavailableReason.empty() ? "The requested setup snapshot does not match the job's latest launch." : output->setupUnavailableReason;
    return false;
  }
  return true;
}

void unavailable(ReportDocument& document, const std::string& section, const std::string& message)
{
  ReportDiagnostic issue;
  issue.section_id = section;
  issue.severity = "unavailable";
  issue.message = message;
  document.diagnostics.push_back(std::move(issue));
}
}

CalculiXReportImageProvider::CalculiXReportImageProvider() = default;
CalculiXReportImageProvider::~CalculiXReportImageProvider() = default;

bool CalculiXReportImageProvider::render(const ReportImageRequest& request, ReportImage& output, std::string& error)
{
  std::shared_ptr<const CalculiXJobRunRecord> archive;
  if (!currentModel(request.source) && !archivedSetup(request.source, archive, error))
  {
    return false;
  }
  CalculiXCaptureRequest capture;
  std::string imageId, caption;
  if (!CalculiXReportImageSettings::decode_capture(request, capture, imageId, caption, error))
  {
    return false;
  }
  QByteArray png;
  const bool rendered = archive ? CalculiXCaptureInterface().render_snapshot(*archive->setup, capture, png, error) :
    CalculiXCaptureInterface().render(capture, png, error);
  if (!rendered)
  {
    return false;
  }
  QBuffer buffer(&png);
  buffer.open(QIODevice::ReadOnly);
  QImageReader reader(&buffer, "png");
  if (png.size() > 32 * 1024 * 1024 || reader.size() != QSize(capture.image.width, capture.image.height) || !reader.canRead())
  {
    error = "The capture provider returned an invalid, oversized or incorrectly sized PNG.";
    return false;
  }
  ReportImage candidate;
  candidate.id = std::move(imageId);
  candidate.caption = std::move(caption);
  candidate.media_type = "image/png";
  candidate.base64_data = png.toBase64().toStdString();
  candidate.provenance = {{"workspace", "calculix"}, {"source_kind", QString::fromStdString(request.source.kind)},
    {"job_id", request.source.job_id}, {"solver_launch_id", QString::fromStdString(request.source.solver_launch_id)},
    {"setup_snapshot_id", QString::fromStdString(request.source.setup_snapshot_id)},
    {"snapshot_fingerprint", QString::fromStdString(request.source.snapshot_fingerprint)},
    {"purpose", QString::fromStdString(request.purpose)}, {"context", QString::fromStdString(request.context)},
    {"width", capture.image.width}, {"height", capture.image.height}};
  output = std::move(candidate);
  error.clear();
  return true;
}

bool CalculiXReportImageProvider::collect(const ReportImageRequest& request, ReportDocument& output, std::string& error)
{
  CalculiXReportImageSettings settings;
  if (!CalculiXReportImageSettings::decode(request.settings, settings, error))
  {
    return false;
  }
  if (request.source.workspace_id != "calculix" || output.source.to_json() != request.source.to_json())
  {
    error = "The image collection source does not match the CalculiX report source.";
    return false;
  }
  ReportDocument candidate = output;
  // This provider owns one root; replacement cannot accumulate old captures or
  // old capture diagnostics. User notes remain in the document's notes map.
  candidate.sections.erase(std::remove_if(candidate.sections.begin(), candidate.sections.end(), [](const ReportSection& section)
  {
    return section.id == "ccx.images";
  }), candidate.sections.end());
  candidate.diagnostics.erase(std::remove_if(candidate.diagnostics.begin(), candidate.diagnostics.end(), [](const ReportDiagnostic& issue)
  {
    return issue.section_id == "ccx.images" || issue.section_id.rfind("ccx.images.", 0) == 0;
  }), candidate.diagnostics.end());
  if (!settings.enabled())
  {
    output = std::move(candidate);
    error.clear();
    return true;
  }
  std::shared_ptr<const CalculiXJobRunRecord> archive;
  std::string archiveError;
  if (!currentModel(request.source) && !archivedSetup(request.source, archive, archiveError))
  {
    unavailable(candidate, "ccx.images", "CalculiX setup images were omitted: " + archiveError + " Current model geometry is not substituted.");
    output = std::move(candidate);
    error.clear();
    return true;
  }
  CalculiXStepsInterface steps;
  std::vector<int> selected;
  const auto selection = request.settings.values.find("steps");
  std::vector<int> ordered;
  if (archive)
  {
    for (const auto& step : archive->setup->steps)
    {
      ordered.push_back(step.stepId);
    }
  }
  else
  {
    ordered = steps.get_steps_ids();
  }
  if ((settings.setup || settings.loads || settings.bcs) &&
      !CalculiXReportImageSettings::select_steps(ordered, selection == request.settings.values.end() ? "all" : selection->second, selected, error))
  {
    return false;
  }
  const std::size_t perStep = static_cast<std::size_t>(settings.setup) + settings.loads + settings.bcs;
  const std::size_t count = static_cast<std::size_t>(settings.model) + perStep * (selected.empty() ? 1 : selected.size());
  if (count > 256)
  {
    error = "A report supports at most 256 CalculiX setup images. Select fewer steps or image types.";
    return false;
  }
  ReportSection root;
  root.id = "ccx.images";
  root.title = "Model and analysis setup images";
  root.paragraphs.push_back(archive ? "Archived exported mesh and frozen reference conditions from this solver launch. Amplitudes are not evaluated; these are not result plots." :
    "Current undeformed mesh and reference load/BC definitions. Amplitudes are not evaluated; these images do not contain calculated results.");
  std::size_t embeddedBytes = 0;
  const auto append = [&](const std::string& purpose, int stepId, ReportSection& section)
  {
    ReportImageRequest imageRequest = request;
    imageRequest.purpose = purpose;
    imageRequest.context = stepId > 0 ? "step:" + std::to_string(stepId) : "model";
    imageRequest.width = settings.width;
    imageRequest.height = settings.height;
    ReportImage image;
    std::string captureError;
    if (!render(imageRequest, image, captureError))
    {
      unavailable(candidate, section.id, purpose + " image unavailable: " + captureError);
      return true;
    }
    embeddedBytes += image.base64_data.size();
    if (embeddedBytes > 128 * 1024 * 1024)
    {
      error = "CalculiX setup images exceed the report's 128 MiB embedded image budget.";
      return false;
    }
    image.caption = section.title + " - " + (purpose == "model" ? "undeformed mesh" : purpose + " reference setup");
    section.images.push_back(std::move(image));
    return true;
  };
  if (settings.model)
  {
    ReportSection model;
    model.id = "ccx.images.model";
    model.title = archive ? "Job model overview" : "Current model overview";
    if (!append("model", -1, model))
    {
      return false;
    }
    root.children.push_back(std::move(model));
  }
  // Models without analysis steps still receive an entire-model setup overview.
  if (selected.empty() && perStep > 0)
  {
    selected.push_back(-1);
  }
  for (int stepId : selected)
  {
    ReportSection step;
    step.id = stepId > 0 ? "ccx.images.steps." + std::to_string(stepId) : "ccx.images.setup";
    step.title = "Current model setup";
    if (stepId > 0)
    {
      StepsSettings data;
      if (archive)
      {
        const auto frozen = std::find_if(archive->setup->steps.begin(), archive->setup->steps.end(), [&](const CalculiXJobSetupScene& item)
        {
          return item.stepId == stepId;
        });
        if (frozen == archive->setup->steps.end())
        {
          error = "The selected step is absent from the archived launch.";
          return false;
        }
        data.name = frozen->name;
      }
      else if (!steps.get_step_settings(stepId, data, error))
      {
        return false;
      }
      const auto position = std::find(ordered.begin(), ordered.end(), stepId);
      step.title = "Analysis step " + std::to_string(std::distance(ordered.begin(), position) + 1) +
        " (ID " + std::to_string(stepId) + "): " + data.name;
    }
    for (const auto& item : {std::pair<const char*, bool>{"setup", settings.setup}, {"loads", settings.loads}, {"bcs", settings.bcs}})
    {
      if (item.second && !append(item.first, stepId, step))
      {
        return false;
      }
    }
    root.children.push_back(std::move(step));
  }
  candidate.sections.push_back(std::move(root));
  output = std::move(candidate);
  error.clear();
  return true;
}
