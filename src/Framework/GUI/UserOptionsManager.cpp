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
  std::vector<WorkspaceOptionsPage> registrations;
  for (const auto& view : candidates)
  {
    if (view.optionsPage != nullptr)
    {
      registrations.push_back({view.label, view.optionsPage, view.refreshOptions, view.applyOptions});
    }
    registrations.insert(registrations.end(), view.additionalOptionsPages.begin(), view.additionalOptionsPages.end());
  }
  for (auto page : registrations)
  {
    // Prefix only registered option labels, never workspace identities or tabs.
    const auto prefix = QStringLiteral("Cubit-Sim - ");
    if (page.label.compare(QStringLiteral("Cubit-Sim"),Qt::CaseInsensitive) == 0)
    {
      page.label = prefix + tr("General");
    }
    else if (!page.label.startsWith(prefix,Qt::CaseInsensitive))
    {
      page.label.prepend(prefix);
    }
    if (page.widget == nullptr || page.label.isEmpty() || !labels.insert(page.label).second || !page.refresh || !page.apply)
    {
      error = "Invalid or duplicate workspace options page.";
      detach();
      return false;
    }
    pages.push_back(page);
    options->add_panel(page.label, page.widget);
  }
  QObject::connect(options.data(), &OptionsPanel::refreshSettings, this, [this]()
  {
    for (const auto& page : pages)
    {
      page.refresh();
    }
  });
  QObject::connect(options.data(), &OptionsPanel::saveSettings, this, [this]()
  {
    for (const auto& page : pages)
    {
      std::string error;
      if (!page.apply(error))
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
