#include "UserOptionsManager.hpp"
#include "Claro.hpp"
#include "OptionsPanel.hpp"
#include "CubitMessage.hpp"
#include <set>

UserOptionsManager::UserOptionsManager() = default;

UserOptionsManager::~UserOptionsManager()
{
  detach();
}

bool UserOptionsManager::attach(const std::vector<WorkspaceProviderInterface*>& providers, std::string& error,
  const std::vector<WorkspaceView>& frameworkPages)
{
  detach();
  auto* gui = Claro::instance();
  if (gui == nullptr || gui->options() == nullptr)
  {
    error = "Cubit has no options dialog.";
    return false;
  }
  options = gui->options();
  std::set<QString> labels;
  auto candidates = frameworkPages;
  for (auto* provider : providers)
  {
    WorkspaceView view;
    if (provider == nullptr || !provider->workspace_view(view, error))
    {
      detach();
      return false;
    }
    candidates.push_back(view);
  }
  for (const auto& view : candidates)
  {
    if (view.optionsPage == nullptr)
    {
      continue;
    }
    if (view.label.isEmpty() || !labels.insert(view.label).second ||
        !view.refreshOptions || !view.applyOptions)
    {
      error = "Invalid or duplicate workspace options page.";
      detach();
      return false;
    }
    pages.push_back(view);
    options->add_panel(view.label, view.optionsPage);
  }
  QObject::connect(options.data(), &OptionsPanel::refreshSettings, this, [this]()
  {
    for (const auto& page : pages)
    {
      page.refreshOptions();
    }
  });
  QObject::connect(options.data(), &OptionsPanel::saveSettings, this, [this]()
  {
    for (const auto& page : pages)
    {
      std::string error;
      if (!page.applyOptions(error))
      {
        PRINT_ERROR("Cannot save %s user options: %s\n",
          page.label.toUtf8().constData(), error.c_str());
      }
    }
  });
  return true;
}

void UserOptionsManager::detach() noexcept
{
  if (options != nullptr)
  {
    QObject::disconnect(options.data(), nullptr, this, nullptr);
    for (const auto& page : pages)
    {
      options->remove_panel(page.label);
    }
  }
  pages.clear();
  options = nullptr;
}
