#ifndef CCXRESULTPLOTFRDJOBCOMMAND_HPP
#define CCXRESULTPLOTFRDJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"
#include <QString>

class ccxResultPlotJobFrdCommand : public CubitCommand
{
public:
  ccxResultPlotJobFrdCommand();
  ~ccxResultPlotJobFrdCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTPLOTFRDJOBCOMMAND_HPP