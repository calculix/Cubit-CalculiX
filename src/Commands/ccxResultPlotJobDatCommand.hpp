#ifndef CCXRESULTPLOTJOBDATCOMMAND_HPP
#define CCXRESULTPLOTJOBDATCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultPlotJobDatCommand : public CubitCommand
{
public:
  ccxResultPlotJobDatCommand();
  ~ccxResultPlotJobDatCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTPLOTJOBDATCOMMAND_HPP