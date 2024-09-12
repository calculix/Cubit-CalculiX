#ifndef CCXRESULTCSVDATJOBCOMMAND_HPP
#define CCXRESULTCSVDATJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"
#include <QString>

class ccxResultCSVJobDatCommand : public CubitCommand
{
public:
  ccxResultCSVJobDatCommand();
  ~ccxResultCSVJobDatCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTCSVDATJOBCOMMAND_HPP