#ifndef CCXTOPOLOGYADDSETSCOMMAND_HPP
#define CCXTOPOLOGYADDSETSCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxTopologyAddSetsCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData& data);
};

#endif