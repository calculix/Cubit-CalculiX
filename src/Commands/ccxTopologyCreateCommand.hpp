#ifndef CCXTOPOLOGYCREATECOMMAND_HPP
#define CCXTOPOLOGYCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxTopologyCreateCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData& data);
};

#endif