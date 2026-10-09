#ifndef CCXRUNTOPOLOGYCOMMAND_HPP
#define CCXRUNTOPOLOGYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief Run a Topology Optimization Job using Open Pisco
 */
class ccxRunTopologyCommand : public CubitCommand
{
public:
  ccxRunTopologyCommand();
  ~ccxRunTopologyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRUNTOPOLOGYCOMMAND_HPP