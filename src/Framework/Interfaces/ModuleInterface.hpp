#ifndef MODULEINTERFACE_HPP
#define MODULEINTERFACE_HPP

#include <string>

class CommandProviderInterface;
class ComponentProviderInterface;
class WorkspaceProviderInterface;

class ModuleInterface
{
public:
  virtual ~ModuleInterface() = default;

  // Stable identifier, independent of translated display text.
  virtual std::string module_id() const = 0;

  virtual bool initialize(std::string& error) = 0;

  // Also valid after partial initialization; implementations must not throw.
  virtual void shutdown() noexcept = 0;

  // Providers are borrowed and remain valid until this module is destroyed.
  virtual CommandProviderInterface* command_provider() noexcept = 0;
  virtual ComponentProviderInterface* component_provider() noexcept = 0;

  virtual WorkspaceProviderInterface* workspace_provider() noexcept
  {
    return nullptr;
  }
};

#endif // MODULEINTERFACE_HPP
