#ifndef CCXRESULTPLOTJOBDATSECTIONCOMMAND_HPP
#define CCXRESULTPLOTJOBDATSECTIONCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultPlotJobDatSectionCommand : public CubitCommand
{
public:
ccxResultPlotJobDatSectionCommand();
  ~ccxResultPlotJobDatSectionCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTPLOTJOBDATSECTIONCOMMAND_HPP