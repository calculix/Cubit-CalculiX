#include "CalculiXReportImageSettings.hpp"
#include <QStringList>
#include <QRegularExpression>
#include <algorithm>
#include <limits>
#include <set>
#include <utility>

namespace
{
const std::vector<std::pair<std::string, DrawingCaptureView>>& captureViews()
{
  static const std::vector<std::pair<std::string, DrawingCaptureView>> views = {{"iso", DrawingCaptureView::Isometric},
    {"front", DrawingCaptureView::Front}, {"back", DrawingCaptureView::Back}, {"left", DrawingCaptureView::Left},
    {"right", DrawingCaptureView::Right}, {"top", DrawingCaptureView::Top}, {"bottom", DrawingCaptureView::Bottom}};
  return views;
}
}

CalculiXReportImageSettings::CalculiXReportImageSettings()
{
  const CalculiXCaptureRequest capture;
  labels = capture.labels.enabled;
  width = capture.image.width;
  height = capture.image.height;
  for (const auto& view : captureViews())
  {
    if (view.second == capture.image.view)
    {
      camera = view.first;
    }
  }
}

bool CalculiXReportImageSettings::enabled() const
{
  return model || setup || loads || bcs;
}

void CalculiXReportImageSettings::add_options(ReportTemplate& definition)
{
  const CalculiXReportImageSettings defaults;
  const auto add = [&](ReportOption option)
  {
    definition.defaults.values[option.key] = option.default_value;
    definition.options.push_back(std::move(option));
  };
  for (const auto& item : std::vector<std::pair<std::string, std::pair<std::string, bool>>>
    {{"model", {"Model overview image", defaults.model}}, {"setup", {"Combined setup image per step", defaults.setup}},
     {"loads", {"Separate load image per step", defaults.loads}}, {"bcs", {"Separate BC image per step", defaults.bcs}},
     {"labels", {"Labels in setup images", defaults.labels}}})
  {
    ReportOption option;
    option.key = "ccx.images." + item.first;
    option.label = item.second.first;
    option.kind = "boolean";
    option.default_value = item.second.second ? "true" : "false";
    add(std::move(option));
  }
  ReportOption view;
  view.key = "ccx.images.camera";
  view.label = "Setup image camera";
  view.kind = "choice";
  for (const auto& camera : captureViews())
  {
    view.choices.push_back(camera.first);
  }
  view.default_value = defaults.camera;
  add(std::move(view));
  for (const auto& item : {std::pair<const char*, int>{"width", defaults.width}, {"height", defaults.height}})
  {
    ReportOption option;
    option.key = std::string("ccx.images.") + item.first;
    option.label = std::string("Setup image ") + item.first + " (pixels)";
    option.kind = "integer";
    option.minimum = 64;
    option.maximum = 8192;
    option.default_value = std::to_string(item.second);
    add(std::move(option));
  }
}

bool CalculiXReportImageSettings::decode(const ReportConfiguration& configuration, CalculiXReportImageSettings& output, std::string& error)
{
  CalculiXReportImageSettings candidate;
  ReportTemplate definition;
  add_options(definition);
  for (const auto& option : definition.options)
  {
    const auto found = configuration.values.find(option.key);
    const std::string value = found == configuration.values.end() ? option.default_value : found->second;
    if (option.kind == "boolean")
    {
      if (value != "true" && value != "false")
      {
        error = "Expected true or false for " + option.key;
        return false;
      }
      const bool flag = value == "true";
      if (option.key == "ccx.images.model")
      {
        candidate.model = flag;
      }
      else if (option.key == "ccx.images.setup")
      {
        candidate.setup = flag;
      }
      else if (option.key == "ccx.images.loads")
      {
        candidate.loads = flag;
      }
      else if (option.key == "ccx.images.bcs")
      {
        candidate.bcs = flag;
      }
      else
      {
        candidate.labels = flag;
      }
    }
    else if (option.kind == "choice")
    {
      if (std::find(option.choices.begin(), option.choices.end(), value) == option.choices.end())
      {
        error = "Unsupported setup image camera.";
        return false;
      }
      candidate.camera = value;
    }
    else
    {
      bool ok = false;
      const int number = QString::fromStdString(value).toInt(&ok);
      if (!ok || number < option.minimum || number > option.maximum)
      {
        error = "Invalid setup image dimension: " + option.key;
        return false;
      }
      if (option.key == "ccx.images.width")
      {
        candidate.width = number;
      }
      else
      {
        candidate.height = number;
      }
    }
  }
  if (static_cast<long long>(candidate.width) * candidate.height > 32000000)
  {
    error = "Setup images support at most 32 million pixels.";
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}

bool CalculiXReportImageSettings::select_steps(const std::vector<int>& orderedIds, const std::string& selection,
  std::vector<int>& output, std::string& error)
{
  std::vector<int> candidate;
  if (selection == "all")
  {
    candidate = orderedIds;
  }
  else
  {
    if (!QRegularExpression("^[1-9][0-9]*(,[1-9][0-9]*)*$").match(QString::fromStdString(selection)).hasMatch())
    {
      error = "Steps must be all or comma-separated one-based ordinals.";
      return false;
    }
    std::set<int> selected;
    for (const auto& value : QString::fromStdString(selection).split(','))
    {
      bool ok = false;
      const int ordinal = value.toInt(&ok);
      if (!ok || static_cast<std::size_t>(ordinal) > orderedIds.size())
      {
        error = "Selected step ordinal does not exist: " + value.toStdString();
        return false;
      }
      selected.insert(ordinal);
    }
    for (std::size_t ordinal = 0; ordinal < orderedIds.size(); ++ordinal)
    {
      if (selected.count(static_cast<int>(ordinal + 1)) != 0)
      {
        candidate.push_back(orderedIds[ordinal]);
      }
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}

bool CalculiXReportImageSettings::decode_capture(const ReportImageRequest& request, CalculiXCaptureRequest& output,
  std::string& imageId, std::string& caption, std::string& error)
{
  CalculiXReportImageSettings settings;
  if (!decode(request.settings, settings, error))
  {
    return false;
  }
  CalculiXCaptureRequest candidate;
  candidate.labels.enabled = settings.labels;
  candidate.image.width = request.width == 0 ? settings.width : request.width;
  candidate.image.height = request.height == 0 ? settings.height : request.height;
  if (candidate.image.width < 64 || candidate.image.width > 8192 || candidate.image.height < 64 || candidate.image.height > 8192 ||
      static_cast<long long>(candidate.image.width) * candidate.image.height > 32000000)
  {
    error = "Invalid report capture dimensions.";
    return false;
  }
  const auto& cameras = captureViews();
  const auto camera = std::find_if(cameras.begin(), cameras.end(), [&](const auto& item)
  {
    return item.first == settings.camera;
  });
  if (camera == cameras.end())
  {
    error = "The selected report camera has no capture implementation.";
    return false;
  }
  candidate.image.view = camera->second;
  const std::map<std::string, CalculiXCaptureContent> contents = {{"model", CalculiXCaptureContent::Model},
    {"setup", CalculiXCaptureContent::Setup}, {"loads", CalculiXCaptureContent::Loads}, {"bcs", CalculiXCaptureContent::BoundaryConditions}};
  const auto content = contents.find(request.purpose);
  if (content == contents.end())
  {
    error = "Unsupported CalculiX report image purpose.";
    return false;
  }
  candidate.content = content->second;
  std::string id = "ccx.images." + request.purpose;
  std::string title = request.purpose == "model" ? "Current model - undeformed mesh" : "Current model - " + request.purpose + " reference setup";
  if (request.context != "model")
  {
    const auto context = QString::fromStdString(request.context);
    bool ok = false;
    const int step = context.mid(5).toInt(&ok);
    if (!QRegularExpression("^step:[1-9][0-9]*$").match(context).hasMatch() || !ok || step <= 0 || request.purpose == "model")
    {
      error = "Invalid CalculiX report image context.";
      return false;
    }
    candidate.scope = {DrawScopeKind::Step, step};
    id = "ccx.images.steps." + std::to_string(step) + "." + request.purpose;
    title = "Step ID " + std::to_string(step) + " - " + request.purpose + " reference setup";
  }
  output = std::move(candidate);
  imageId = std::move(id);
  caption = std::move(title);
  error.clear();
  return true;
}
