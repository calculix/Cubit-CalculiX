#ifndef CCXRESULTCSVFRDJOBCOMMAND_HPP
#define CCXRESULTCSVFRDJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"
#include <QString>

class ccxResultCSVJobFrdCommand : public CubitCommand
{
public:
  ccxResultCSVJobFrdCommand();
  ~ccxResultCSVJobFrdCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTCSVFRDJOBCOMMAND_HPP