#ifndef CCXRESULTCONVERTJOBCOMMAND_HPP
#define CCXRESULTCONVERTJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultConvertJobCommand : public CubitCommand
{
public:
  ccxResultConvertJobCommand();
  ~ccxResultConvertJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTCONVERTJOBCOMMAND_HPP