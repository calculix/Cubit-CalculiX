#ifndef POSTPROCESSORCOMMANDS_HPP
#define POSTPROCESSORCOMMANDS_HPP

#include "Framework/Interfaces/CommandProviderInterface.hpp"

#include <memory>

class PostprocessorApplicationInterface;

class PostprocessorCommands : public CommandProviderInterface
{
public:
  explicit PostprocessorCommands(std::weak_ptr<PostprocessorApplicationInterface> application);
  std::vector<std::string> command_keys() const override;
  CubitCommand* create_command(const std::string& key) override;

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};

#endif // POSTPROCESSORCOMMANDS_HPP
