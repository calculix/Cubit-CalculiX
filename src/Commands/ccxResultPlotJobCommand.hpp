#ifndef CCXRESULTPLOTJOBCOMMAND_HPP
#define CCXRESULTPLOTJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultPlotJobCommand : public CubitCommand
{
public:
  ccxResultPlotJobCommand();
  ~ccxResultPlotJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTPLOTJOBCOMMAND_HPP