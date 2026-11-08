#ifndef CCXRESULTEXPORTJOBCOMMAND_HPP
#define CCXRESULTEXPORTJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultExportJobCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData& data);
};

#endif
