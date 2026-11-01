#ifndef CCXINITPYTHONINTERFACECOMMAND_HPP
#define CCXINITPYTHONINTERFACECOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxInitPythonInterfaceCommand : public CubitCommand
{
public:
  ccxInitPythonInterfaceCommand();
  ~ccxInitPythonInterfaceCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXINITPYTHONINTERFACECOMMAND_HPP
