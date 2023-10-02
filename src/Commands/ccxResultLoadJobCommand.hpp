#ifndef CCXRESULTLOADJOBCOMMAND_HPP
#define CCXRESULTLOADJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxResultLoadJobCommand : public CubitCommand
{
public:
  ccxResultLoadJobCommand();
  ~ccxResultLoadJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESULTLOADJOBCOMMAND_HPP