#include "PostprocessorToolbar.hpp"
#include "Claro.hpp"
#include "ClaroFileMenu.hpp"
#include "ClaroToolsMenu.hpp"
#include <QAction>
#include <QKeySequence>
#include <vector>

namespace
{
QAction* standardAction(const std::vector<QAction*>& actions, QKeySequence::StandardKey key)
{
  const auto bindings = QKeySequence::keyBindings(key);
  QAction* found = nullptr;
  for (auto* action : actions)
  {
    if (action == nullptr || action->isSeparator())
    {
      continue;
    }
    bool matches = false;
    for (const auto& shortcut : action->shortcuts())
    {
      matches = matches || bindings.contains(shortcut);
    }
    if (matches)
    {
      if (found != nullptr && found != action)
      {
        return nullptr;
      }
      found = action;
    }
  }
  return found;
}
}

PostprocessorToolbar::PostprocessorToolbar(QWidget* parent) : QToolBar(tr("Project and journal tools"),parent)
{
  setObjectName("Postprocessor.NativeToolbar");
  setIconSize(QSize(24,24));
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  setAllowedAreas(Qt::TopToolBarArea);
  setMovable(false);
  setFloatable(false);
}

bool PostprocessorToolbar::bind_native_actions(Claro* host, std::string& error)
{
  if (nativeBound)
  {
    error.clear();
    return true;
  }
  if (host == nullptr || host->file_menu() == nullptr || host->tools_menu() == nullptr)
  {
    error = "Cannot access Cubit's native file and journal actions.";
    return false;
  }
  std::vector<QAction*> files;
  host->file_menu()->get_items_for_toolbar(files);
  QList<QAction*> selected;
  for (const auto key : {QKeySequence::New, QKeySequence::Open, QKeySequence::Save})
  {
    auto* action = standardAction(files, key);
    if (action == nullptr || selected.contains(action))
    {
      error = "Cannot uniquely identify Cubit's native New, Open and Save toolbar actions by their standard shortcuts.";
      return false;
    }
    selected.append(action);
    if (key == QKeySequence::Open)
    {
      openIcon = action->icon();
    }
  }
  // Borrow the original actions without cloning, reparenting or changing their
  // shortcuts/enabled states. Cubit continues to own their handlers and icons.
  addActions(selected);
  addSeparator();
  std::vector<QAction*> journals;
  host->tools_menu()->get_items_for_toolbar(journals);
  if (!journals.empty())
  {
    for (auto* action : journals)
    {
      if (action != nullptr)
      {
        addAction(action);
      }
    }
    addSeparator();
  }
  nativeBound = true;
  error.clear();
  return true;
}


QIcon PostprocessorToolbar::open_icon() const
{
  return openIcon;
}
