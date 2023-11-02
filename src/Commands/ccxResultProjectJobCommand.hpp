#ifndef CCXRESULTPROJECTJOBCOMMAND_HPP
#define CCXRESULTPROJECTJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultProjectJobCommand : public CubitCommand
{
public:
  ccxResultProjectJobCommand();
  ~ccxResultProjectJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTPROJECTJOBCOMMAND_HPP