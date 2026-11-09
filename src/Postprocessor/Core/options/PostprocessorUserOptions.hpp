#ifndef POSTPROCESSORUSEROPTIONS_HPP
#define POSTPROCESSORUSEROPTIONS_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include "Postprocessor/Core/representations/DisplayDefaults.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"

class UserOptionsInterface;

namespace Postprocessor
{
// RGB text is a portable preferences value; each finite component is in [0,1].
bool parse_background_color(const std::string& text, std::array<double, 3>& color);
std::string format_background_color(const std::array<double, 3>& color);

enum class MouseDragAction
{
  Rotate,
  Zoom,
  Pan
};
// Exact stable command/storage names; UI translations must not change these.
bool parse_mouse_drag_action(const std::string& text, MouseDragAction& action);
const char* mouse_drag_action_name(MouseDragAction action);

namespace MouseDefaults
{
  inline constexpr MouseDragAction left = MouseDragAction::Rotate;
  inline constexpr MouseDragAction middle = MouseDragAction::Zoom;
  inline constexpr MouseDragAction right = MouseDragAction::Pan;
}

struct UserOptions
{
  std::string iconsPath;
  bool journalCamera = false;
  bool journalNavigation = true;
  bool diagnosticsCamera = false;
  std::array<double, 3> backgroundTop = DisplayDefaults::background_top;
  std::array<double, 3> backgroundBottom = DisplayDefaults::background_bottom;
  bool triadVisible = DisplayDefaults::triad_visible;
  ColorPalette resultPalette = ColoringDefaults::palette;
  double mouseSensitivity = DisplayDefaults::mouse_sensitivity;
  bool reverseZoom = DisplayDefaults::reverse_zoom;
  MouseDragAction leftDrag = MouseDefaults::left;
  MouseDragAction middleDrag = MouseDefaults::middle;
  MouseDragAction rightDrag = MouseDefaults::right;
};

class PostprocessorUserOptions
{
public:
  static std::vector<UserOption> definitions();
  bool initialize(UserOptionsInterface& store, std::string& error);
  void detach() noexcept;
  const UserOptions& current() const noexcept;
  const std::string& active_icons_path() const noexcept;
  bool restart_required() const;
  bool values(UserOptionValues& snapshot, std::string& error) const;
  // Typed edits are validated and merged before saving and publication.
  // Failure leaves both saved configuration and active session values unchanged.
  bool apply(const UserOptionValues& changes, std::string& error);

private:
  UserOptionsInterface* store = nullptr;
  UserOptions configured;
  std::string activeIconsPath;
};
}

#endif
