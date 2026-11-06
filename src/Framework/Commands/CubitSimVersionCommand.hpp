#ifndef CUBITSIM_VERSIONCOMMAND_HPP
#define CUBITSIM_VERSIONCOMMAND_HPP

#include "CubitCommandInterface.hpp"

// Reports the application and host runtime; no workspace interface is needed.
class CubitSimVersionCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;
};

#endif
