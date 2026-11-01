#ifndef CALCULIXCOMMANDS_HPP
#define CALCULIXCOMMANDS_HPP

#include "Framework/Interfaces/CommandProviderInterface.hpp"

class CalculiXCommands : public CommandProviderInterface
{
public:
  std::vector<std::string> command_keys() const override;
  CubitCommand* create_command(const std::string& key) override;
};

#endif // CALCULIXCOMMANDS_HPP
