#include "PostprocessorUserOptions.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"

#include <QDir>
#include <QFileInfo>
#include <exception>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace
{
bool validPath(const UserOptionValue& value)
{
  const auto* path = std::get_if<std::string>(&value);
  if (path == nullptr || path->empty() || path->find('\0') != std::string::npos || path->find_first_of("\"\r\n") != std::string::npos)
  {
    return false;
  }
  const QString normalized = QDir::cleanPath(QDir::fromNativeSeparators(QString::fromUtf8(path->c_str())));
  return QFileInfo(normalized).isAbsolute() ||
    (normalized != ".." && !normalized.startsWith("../"));
}

QString resolvedPath(const std::string& directory, const std::string& path)
{
  return QDir::cleanPath(QDir(QString::fromUtf8(directory.c_str())).absoluteFilePath(
    QDir::fromNativeSeparators(QString::fromUtf8(path.c_str()))));
}
}

bool Postprocessor::parse_background_color(const std::string& text, std::array<double, 3>& color)
{
  std::istringstream stream(text);
  stream.imbue(std::locale::classic());
  std::array<double, 3> candidate;
  for (auto& component : candidate)
  {
    if (!(stream >> component) || !std::isfinite(component) || component < 0 || component > 1)
    {
      return false;
    }
  }
  stream >> std::ws;
  if (!stream.eof())
  {
    return false;
  }
  color = candidate;
  return true;
}

std::string Postprocessor::format_background_color(const std::array<double, 3>& color)
{
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  stream << std::setprecision(std::numeric_limits<double>::max_digits10)
    << color[0] << ' ' << color[1] << ' ' << color[2];
  return stream.str();
}

bool Postprocessor::parse_mouse_drag_action(const std::string& text, MouseDragAction& action)
{
  if (text == "rotate")
  {
    action = MouseDragAction::Rotate;
  }
  else if (text == "zoom")
  {
    action = MouseDragAction::Zoom;
  }
  else if (text == "pan")
  {
    action = MouseDragAction::Pan;
  }
  else
  {
    return false;
  }
  return true;
}

const char* Postprocessor::mouse_drag_action_name(MouseDragAction action)
{
  switch (action)
  {
    case MouseDragAction::Rotate:
      return "rotate";
    case MouseDragAction::Zoom:
      return "zoom";
    case MouseDragAction::Pan:
      return "pan";
  }
  return "";
}

std::vector<UserOption> Postprocessor::PostprocessorUserOptions::definitions()
{
  // Authoritative defaults live in Core. Add future display defaults here;
  // application preferences supply defaults, while individual views own overrides.
  const auto validAction = [](const UserOptionValue& value)
  {
    const auto* text = std::get_if<std::string>(&value);
    MouseDragAction action;
    return text && parse_mouse_drag_action(*text, action);
  };
  return {
    {"mouse/left", UserOptionKind::Text, std::string(mouse_drag_action_name(MouseDefaults::left)), validAction},
    {"mouse/middle", UserOptionKind::Text, std::string(mouse_drag_action_name(MouseDefaults::middle)), validAction},
    {"mouse/right", UserOptionKind::Text, std::string(mouse_drag_action_name(MouseDefaults::right)), validAction},
    {"paths/icons", UserOptionKind::ComponentPath, std::string("Icons/"), validPath},
    {"journal/camera", UserOptionKind::Boolean, false},
    {"diagnostics/camera", UserOptionKind::Boolean, false},
    {"background/top", UserOptionKind::Text, format_background_color(DisplayDefaults::background_top), [](const UserOptionValue& value)
    {
      std::array<double, 3> color;
      const auto* text = std::get_if<std::string>(&value);
      return text && parse_background_color(*text, color);
    }},
    {"background/bottom", UserOptionKind::Text, format_background_color(DisplayDefaults::background_bottom), [](const UserOptionValue& value)
    {
      std::array<double, 3> color;
      const auto* text = std::get_if<std::string>(&value);
      return text && parse_background_color(*text, color);
    }},
    {"display/triad", UserOptionKind::Boolean, DisplayDefaults::triad_visible},
    {"mouse/sensitivity", UserOptionKind::Real, DisplayDefaults::mouse_sensitivity, [](const UserOptionValue& value)
    {
      const auto* gain = std::get_if<double>(&value);
      return gain && std::isfinite(*gain) && *gain >= DisplayDefaults::sensitivity_min && *gain <= DisplayDefaults::sensitivity_max;
    }},
    {"mouse/reversezoom", UserOptionKind::Boolean, DisplayDefaults::reverse_zoom}
  };
}

bool Postprocessor::PostprocessorUserOptions::initialize(UserOptionsInterface& options, std::string& error)
{
  error.clear();
  try
  {
    UserOptionValues values;
    if (!options.load("postprocessor", definitions(), values, error))
    {
      return false;
    }
    auto path = std::get<std::string>(values.at("paths/icons"));
    auto active = resolvedPath(options.component_directory(), path).toUtf8().toStdString();
    UserOptions candidate;
    candidate.journalCamera = std::get<bool>(values.at("journal/camera"));
    candidate.diagnosticsCamera = std::get<bool>(values.at("diagnostics/camera"));
    candidate.triadVisible = std::get<bool>(values.at("display/triad"));
    candidate.mouseSensitivity = std::get<double>(values.at("mouse/sensitivity"));
    candidate.reverseZoom = std::get<bool>(values.at("mouse/reversezoom"));
    if (!parse_mouse_drag_action(std::get<std::string>(values.at("mouse/left")), candidate.leftDrag) ||
        !parse_mouse_drag_action(std::get<std::string>(values.at("mouse/middle")), candidate.middleDrag) ||
        !parse_mouse_drag_action(std::get<std::string>(values.at("mouse/right")), candidate.rightDrag))
    {
      error = "Mouse drag assignments must be rotate, zoom or pan.";
      return false;
    }
    if (!parse_background_color(std::get<std::string>(values.at("background/top")), candidate.backgroundTop) ||
        !parse_background_color(std::get<std::string>(values.at("background/bottom")), candidate.backgroundBottom))
    {
      error = "Background colors require three RGB components in [0,1].";
      return false;
    }
    configured.iconsPath.swap(path);
    configured.journalCamera = candidate.journalCamera;
    configured.diagnosticsCamera = candidate.diagnosticsCamera;
    configured.backgroundTop = candidate.backgroundTop;
    configured.backgroundBottom = candidate.backgroundBottom;
    configured.triadVisible = candidate.triadVisible;
    configured.mouseSensitivity = candidate.mouseSensitivity;
    configured.reverseZoom = candidate.reverseZoom;
    configured.leftDrag = candidate.leftDrag;
    configured.middleDrag = candidate.middleDrag;
    configured.rightDrag = candidate.rightDrag;
    activeIconsPath.swap(active);
    store = &options;
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

void Postprocessor::PostprocessorUserOptions::detach() noexcept
{
  store = nullptr;
}

const Postprocessor::UserOptions& Postprocessor::PostprocessorUserOptions::current() const noexcept
{
  return configured;
}

const std::string& Postprocessor::PostprocessorUserOptions::active_icons_path() const noexcept
{
  return activeIconsPath;
}

bool Postprocessor::PostprocessorUserOptions::restart_required() const
{
  if (store == nullptr)
  {
    return false;
  }
  const auto saved = resolvedPath(store->component_directory(), configured.iconsPath);
  const auto active = QString::fromUtf8(activeIconsPath.c_str());
#ifdef _WIN32
  return saved.compare(active, Qt::CaseInsensitive) != 0;
#else
  return saved != active;
#endif
}

bool Postprocessor::PostprocessorUserOptions::apply(const UserOptionValues& changes, std::string& error)
{
  error.clear();
  if (store == nullptr)
  {
    error = "Postprocessor user options are unavailable.";
    return false;
  }
  try
  {
    const auto schema = definitions();
    UserOptionValues values;
    if (!store->load("postprocessor", schema, values, error) ||
      !merge_user_option_changes(schema, changes, values, error))
    {
      return false;
    }
    auto path = std::get<std::string>(values.at("paths/icons"));
    const auto normalized = QDir::fromNativeSeparators(QString::fromUtf8(path.c_str()));
    if (QFileInfo(normalized).isAbsolute())
    {
      const auto relative = QDir(QString::fromUtf8(store->component_directory().c_str())).relativeFilePath(QDir::cleanPath(normalized));
      if (relative != ".." && !relative.startsWith("../") && !QFileInfo(relative).isAbsolute())
      {
        path = relative.toUtf8().toStdString();
      }
    }
    values["paths/icons"] = path;
    UserOptions candidate;
    candidate.journalCamera = std::get<bool>(values.at("journal/camera"));
    candidate.diagnosticsCamera = std::get<bool>(values.at("diagnostics/camera"));
    candidate.triadVisible = std::get<bool>(values.at("display/triad"));
    candidate.mouseSensitivity = std::get<double>(values.at("mouse/sensitivity"));
    candidate.reverseZoom = std::get<bool>(values.at("mouse/reversezoom"));
    if (!parse_mouse_drag_action(std::get<std::string>(values.at("mouse/left")), candidate.leftDrag) ||
        !parse_mouse_drag_action(std::get<std::string>(values.at("mouse/middle")), candidate.middleDrag) ||
        !parse_mouse_drag_action(std::get<std::string>(values.at("mouse/right")), candidate.rightDrag))
    {
      error = "Mouse drag assignments must be rotate, zoom or pan.";
      return false;
    }
    if (!parse_background_color(std::get<std::string>(values.at("background/top")), candidate.backgroundTop) ||
        !parse_background_color(std::get<std::string>(values.at("background/bottom")), candidate.backgroundBottom))
    {
      error = "Background colors require three RGB components in [0,1].";
      return false;
    }
    if (!store->save("postprocessor", schema, values, error))
    {
      return false;
    }
    // Non-throwing publication after persistence; active icons wait for restart.
    configured.iconsPath.swap(path);
    configured.journalCamera = candidate.journalCamera;
    configured.diagnosticsCamera = candidate.diagnosticsCamera;
    configured.backgroundTop = candidate.backgroundTop;
    configured.backgroundBottom = candidate.backgroundBottom;
    configured.triadVisible = candidate.triadVisible;
    configured.mouseSensitivity = candidate.mouseSensitivity;
    configured.reverseZoom = candidate.reverseZoom;
    configured.leftDrag = candidate.leftDrag;
    configured.middleDrag = candidate.middleDrag;
    configured.rightDrag = candidate.rightDrag;
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

bool Postprocessor::PostprocessorUserOptions::values(UserOptionValues& snapshot, std::string& error) const
{
  if (store == nullptr)
  {
    error = "Postprocessor user options are unavailable.";
    return false;
  }
  return store->load("postprocessor", definitions(), snapshot, error);
}
