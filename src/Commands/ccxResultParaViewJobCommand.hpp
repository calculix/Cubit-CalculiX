#ifndef CCXRESULTPARAVIEWJOBCOMMAND_HPP
#define CCXRESULTPARAVIEWJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultParaViewJobCommand : public CubitCommand
{
public:
  ccxResultParaViewJobCommand();
  ~ccxResultParaViewJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTPARAVIEWJOBCOMMAND_HPP