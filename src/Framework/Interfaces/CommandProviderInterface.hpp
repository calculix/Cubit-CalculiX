#ifndef COMMANDPROVIDERINTERFACE_HPP
#define COMMANDPROVIDERINTERFACE_HPP

#include <string>
#include <vector>

class CubitCommand;

class CommandProviderInterface
{
public:
  virtual ~CommandProviderInterface() = default;

  // Keys remain fixed for the lifetime of a registered module.
  virtual std::vector<std::string> command_keys() const = 0;

  // Cubit takes ownership of the returned command.
  virtual CubitCommand* create_command(const std::string& key) = 0;
};

#endif // COMMANDPROVIDERINTERFACE_HPP
