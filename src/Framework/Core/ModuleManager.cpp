#include "ModuleManager.hpp"
#include "Framework/Interfaces/CommandProviderInterface.hpp"
#include "Framework/Interfaces/ComponentProviderInterface.hpp"
#include "Framework/Interfaces/ModuleInterface.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"

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

bool ModuleManager::initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error)
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

      if (!module->initialize(options, drawings, error))
      {
        if (error.empty())
        {
          error = "Module initialization failed: " + module->module_id();
        }

        shutdown();
        return false;
      }

      // Result providers are independent of command and GUI providers.
      if (auto* result_provider = module->result_source_provider())
      {
        const std::string prefix = result_provider->command_prefix();

        if (prefix.empty() ||
            !result_providers.emplace(prefix, result_provider).second ||
            !application_result_providers.emplace(module->module_id(), result_provider).second)
        {
          error = "Empty or duplicate result-provider registration in module: "
            + module->module_id();
          shutdown();
          return false;
        }
      }

      auto* provider = module->command_provider();

      if (provider == nullptr)
      {
        continue;
      }

      for (const auto& key : provider->command_keys())
      {
        if (key.empty() || key == "CubitSimWorkspaceCommand" || key == "CubitSimDocumentationCommand" ||
            key == "CubitSimVersionCommand" || !command_owners.emplace(key, provider).second)
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
  result_providers.clear();
  application_result_providers.clear();
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

ResultSourceProviderInterface* ModuleManager::result_source_provider(const std::string& prefix) const noexcept
{
  const auto provider = result_providers.find(prefix);

  if (!initialized || provider == result_providers.end())
  {
    return nullptr;
  }

  return provider->second;
}

ResultSourceProviderInterface* ModuleManager::application_result_source_provider(const std::string& application_id) const noexcept
{
  const auto provider = application_result_providers.find(application_id);

  if (!initialized || provider == application_result_providers.end())
  {
    return nullptr;
  }

  return provider->second;
}

bool ModuleManager::resolve_result_provider(
  const std::string& prefix,
  ResolvedResultSourceProvider& output,
  std::string& error) const
{
  error.clear();

  try
  {
    auto* provider = result_source_provider(prefix);

    if (provider == nullptr)
    {
      error = "Result provider is unavailable or unknown: " + prefix;
      return false;
    }

    for (const auto& application : application_result_providers)
    {
      if (application.second == provider)
      {
        ResolvedResultSourceProvider resolved;
        resolved.application_id = application.first;
        resolved.provider = provider;
        output = std::move(resolved);
        return true;
      }
    }

    error = "Result provider has no application identity: " + prefix;
    return false;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot resolve result provider: ") + exception.what();
    return false;
  }
  catch (...)
  {
    error = "Cannot resolve result provider: unknown failure.";
    return false;
  }
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
