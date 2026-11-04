#include "ModuleManager.hpp"
#include "Framework/Interfaces/CommandProviderInterface.hpp"
#include "Framework/Interfaces/ComponentProviderInterface.hpp"
#include "Framework/Interfaces/ModuleInterface.hpp"

#include <exception>
#include <utility>

ModuleManager::ModuleManager() = default;

ModuleManager::~ModuleManager()
{
  shutdown();
}

bool ModuleManager::add_module(
  std::unique_ptr<ModuleInterface> module,
  std::string& error)
{
  error.clear();

  if (registration_closed || !module)
  {
    error = "Module registration is closed or the module is null.";
    return false;
  }

  const std::string id = module->module_id();

  if (id.empty())
  {
    error = "A module must have a non-empty identifier.";
    return false;
  }

  for (const auto& existing : modules)
  {
    if (existing->module_id() == id)
    {
      error = "Duplicate module identifier: " + id;
      return false;
    }
  }

  modules.push_back(std::move(module));
  return true;
}

bool ModuleManager::initialize(UserOptionsInterface& options, std::string& error)
{
  error.clear();

  if (initialized)
  {
    return true;
  }

  // The module set cannot change after Cubit has requested its commands.
  registration_closed = true;

  try
  {
    started_modules.reserve(modules.size());

    for (const auto& module : modules)
    {
      // Record before calling initialize so partial startup is cleaned up too.
      started_modules.push_back(module.get());

      if (!module->initialize(options, error))
      {
        if (error.empty())
        {
          error = "Module initialization failed: " + module->module_id();
        }

        shutdown();
        return false;
      }

      auto* provider = module->command_provider();

      if (provider == nullptr)
      {
        continue;
      }

      for (const auto& key : provider->command_keys())
      {
        if (key.empty() || key == "feaWorkspaceCommand" || !command_owners.emplace(key, provider).second)
        {
          error = "Empty, reserved or duplicate command key in module: "
            + module->module_id() + " (" + key + ")";
          shutdown();
          return false;
        }

        ordered_keys.push_back(key);
      }
    }
  }
  catch (const std::exception& exception)
  {
    error = std::string("Module initialization failed: ") + exception.what();
    shutdown();
    return false;
  }
  catch (...)
  {
    error = "Module initialization failed with an unknown exception.";
    shutdown();
    return false;
  }

  initialized = true;
  return true;
}

void ModuleManager::shutdown() noexcept
{
  stop_components();
  command_owners.clear();
  ordered_keys.clear();

  for (auto module = started_modules.rbegin(); module != started_modules.rend(); ++module)
  {
    (*module)->shutdown();
  }

  started_modules.clear();
  initialized = false;
}

bool ModuleManager::start_components(int withGUI, QWidget* host, std::string& error)
{
  error.clear();

  if (!initialized || components_started)
  {
    error = "Component startup needs an initialized framework and no active component owner.";
    return false;
  }

  try
  {
    started_components.reserve(modules.size());

    for (const auto& module : modules)
    {
      auto* provider = module->component_provider();

      if (provider == nullptr)
      {
        continue;
      }

      started_components.push_back(provider);

      if (!provider->start_component(withGUI, host, error))
      {
        if (error.empty())
        {
          error = "Component startup failed: " + module->module_id();
        }

        stop_components();
        return false;
      }
    }
  }
  catch (const std::exception& exception)
  {
    error = std::string("Component startup failed: ") + exception.what();
    stop_components();
    return false;
  }
  catch (...)
  {
    error = "Component startup failed with an unknown exception.";
    stop_components();
    return false;
  }

  components_started = true;
  return true;
}

void ModuleManager::stop_components() noexcept
{
  for (auto provider = started_components.rbegin(); provider != started_components.rend(); ++provider)
  {
    (*provider)->stop_component();
  }

  started_components.clear();
  components_started = false;
}

std::vector<std::string> ModuleManager::command_keys() const
{
  return ordered_keys;
}

CubitCommand* ModuleManager::create_command(const std::string& key)
{
  const auto owner = command_owners.find(key);

  if (!initialized || owner == command_owners.end())
  {
    return nullptr;
  }

  return owner->second->create_command(key);
}

std::vector<WorkspaceProviderInterface*> ModuleManager::workspace_providers() const
{
  std::vector<WorkspaceProviderInterface*> providers;

  for (auto* module : started_modules)
  {
    if (auto* provider = module->workspace_provider())
    {
      providers.push_back(provider);
    }
  }

  return providers;
}
