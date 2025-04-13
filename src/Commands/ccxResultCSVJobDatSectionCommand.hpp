#ifndef CCXRESULTCSVDATJOBSECTIONCOMMAND_HPP
#define CCXRESULTCSVDATJOBSECTIONCOMMAND_HPP

#include "CubitCommandInterface.hpp"
#include <QString>

class ccxResultCSVJobDatSectionCommand : public CubitCommand
{
public:
  ccxResultCSVJobDatSectionCommand();
  ~ccxResultCSVJobDatSectionCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTCSVDATJOBSECTIONCOMMAND_HPP