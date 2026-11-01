#ifndef WORKSPACECOMMAND_HPP
#define WORKSPACECOMMAND_HPP

#include "CubitCommandInterface.hpp"

class WorkspaceCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;
};

#endif // WORKSPACECOMMAND_HPP
