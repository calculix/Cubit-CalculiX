#ifndef CCXANIMATETOPOLOGYCOMMAND_HPP
#define CCXANIMATETOPOLOGYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

class ccxAnimateTopologyCommand : public CubitCommand
{
public:
  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData& data);
};

#endif