#ifndef CCXUSELOGCORECOMMAND_HPP
#define CCXUSELOGCORECOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxUseLogCoreCommand : public CubitCommand
{
public:
  ccxUseLogCoreCommand();
  ~ccxUseLogCoreCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXUSELOGCORECOMMAND_HPP
