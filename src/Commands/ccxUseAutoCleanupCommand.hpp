#ifndef CCXUSEAUTOCLEANUPCOMMAND_HPP
#define CCXUSEAUTOCLEANUPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxUseAutoCleanupCommand : public CubitCommand
{
public:
  ccxUseAutoCleanupCommand();
  ~ccxUseAutoCleanupCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXUSEAUTOCLEANUPCOMMAND_HPP