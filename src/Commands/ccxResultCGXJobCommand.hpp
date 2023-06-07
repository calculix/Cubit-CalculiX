#ifndef CCXRESULTCGXJOBCOMMAND_HPP
#define CCXRESULTCGXJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultCGXJobCommand : public CubitCommand
{
public:
  ccxResultCGXJobCommand();
  ~ccxResultCGXJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTCGXJOBCOMMAND_HPP