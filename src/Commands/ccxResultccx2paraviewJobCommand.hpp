#ifndef CCXRESULTCCX2PARAVIEWJOBCOMMAND_HPP
#define CCXRESULTCCX2PARAVIEWJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultccx2paraviewJobCommand : public CubitCommand
{
public:
  ccxResultccx2paraviewJobCommand();
  ~ccxResultccx2paraviewJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTCCX2PARAVIEWJOBCOMMAND_HPP