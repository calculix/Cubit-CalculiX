#include "ccxCaptureCommand.hpp"
#include "Core/drawing/CalculiXCaptureInterface.hpp"
#include "CubitMessage.hpp"
#include <QFileInfo>
#include <QSaveFile>
#include <cmath>
#include <utility>

std::vector<std::string> ccxCaptureCommand::get_syntax()
{
  return {"ccx capture {model|setup|loads|bcs}"
    " [step <value:label='capture_step'>]"
    " [view {iso|front|back|left|right|top|bottom}]"
    " [labels {on|off}]"
    " [width <value:label='capture_width'>]"
    " [height <value:label='capture_height'>]"
    " file <string:label='capture_file'> [overwrite]"};
}

std::vector<std::string> ccxCaptureCommand::get_syntax_help()
{
  return {"ccx capture {model|setup|loads|bcs} [step <id>] [view {iso|front|back|left|right|top|bottom}]"
    " [labels {on|off}] [width <pixels>] [height <pixels>] file \"<path.png>\" [overwrite]"};
}

std::vector<std::string> ccxCaptureCommand::get_help()
{
  return {"Capture the current undeformed meshed model and its load/BC setup without changing the visible workspace."
    " Defaults: 1920 x 1080, white background, fitted isometric view and black labels."
    " Step uses resolved reference conditions; amplitudes are not evaluated."
    " Dimensions must be integers in 64..8192, at most 32 million pixels."
    " Existing PNG files require overwrite. This does not capture a historical job model or results."};
}

bool ccxCaptureCommand::execute(CubitCommandData& data)
{
  CalculiXCaptureRequest request;
  if (data.find_keyword("MODEL"))
  {
    request.content = CalculiXCaptureContent::Model;
  }
  else if (data.find_keyword("LOADS"))
  {
    request.content = CalculiXCaptureContent::Loads;
  }
  else if (data.find_keyword("BCS"))
  {
    request.content = CalculiXCaptureContent::BoundaryConditions;
  }
  const auto integer = [&](const char* label, int& result, int minimum, int maximum)
  {
    double value = result;
    if (!data.get_value(label, value) || !std::isfinite(value) || value != std::floor(value) || value < minimum || value > maximum)
    {
      PRINT_ERROR("Capture requires an integer between %d and %d for %s.\n", minimum, maximum, label);
      return false;
    }
    result = static_cast<int>(value);
    return true;
  };
  if (data.find_keyword("STEP"))
  {
    request.scope.kind = DrawScopeKind::Step;
    if (!integer("capture_step", request.scope.stepId, 1, 2147483647))
    {
      return false;
    }
  }
  if ((data.find_keyword("WIDTH") && !integer("capture_width", request.image.width, 64, 8192)) ||
      (data.find_keyword("HEIGHT") && !integer("capture_height", request.image.height, 64, 8192)))
  {
    return false;
  }
  if (static_cast<long long>(request.image.width) * request.image.height > 32000000)
  {
    PRINT_ERROR("Capture supports at most 32 million pixels.\n");
    return false;
  }
  for (const auto& view : {std::pair<const char*, DrawingCaptureView>{"ISO", DrawingCaptureView::Isometric},
      {"FRONT", DrawingCaptureView::Front}, {"BACK", DrawingCaptureView::Back}, {"LEFT", DrawingCaptureView::Left},
      {"RIGHT", DrawingCaptureView::Right}, {"TOP", DrawingCaptureView::Top}, {"BOTTOM", DrawingCaptureView::Bottom}})
  {
    if (data.find_keyword(view.first))
    {
      request.image.view = view.second;
    }
  }
  if (data.find_keyword("LABELS"))
  {
    request.labels.enabled = data.find_keyword("ON");
  }
  std::string filename;
  if (!data.get_string("capture_file", filename) || filename.empty())
  {
    PRINT_ERROR("Capture requires a PNG file path.\n");
    return false;
  }
  const auto path = QString::fromStdString(filename);
  const QFileInfo destination(path);
  const bool overwrite = data.find_keyword("OVERWRITE");
  if (destination.suffix().compare("png", Qt::CaseInsensitive) != 0 || destination.isDir())
  {
    PRINT_ERROR("Capture output must be a PNG file.\n");
    return false;
  }
  if (!overwrite && (destination.exists() || destination.isSymLink()))
  {
    PRINT_ERROR("Capture file exists. Add overwrite to replace it.\n");
    return false;
  }
  QByteArray png;
  std::string error;
  if (!CalculiXCaptureInterface().render(request, png, error))
  {
    PRINT_ERROR("Capture failed: %s\n", error.c_str());
    return false;
  }
  // Recheck after rendering; QSaveFile keeps an existing file intact on errors.
  const QFileInfo current(path);
  if (!overwrite && (current.exists() || current.isSymLink()))
  {
    PRINT_ERROR("Capture file appeared during rendering. Add overwrite to replace it.\n");
    return false;
  }
  QSaveFile file(path);
  file.setDirectWriteFallback(false);
  if (!file.open(QIODevice::WriteOnly) || file.write(png) != png.size() || !file.commit())
  {
    PRINT_ERROR("Cannot save capture: %s\n", file.errorString().toStdString().c_str());
    return false;
  }
  PRINT_INFO("Saved CalculiX capture %s (%d x %d).\n", filename.c_str(), request.image.width, request.image.height);
  return true;
}
