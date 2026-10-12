#ifndef CCXTOPOLOGYDELETECOMMAND_HPP
#define CCXTOPOLOGYDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxTopologyDeleteCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData& data);
};

#endif