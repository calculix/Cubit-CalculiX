#ifndef CCXCONSTRAINTEQUATIONCREATEFROMCOINCIDENTNODESCOMMAND_HPP
#define CCXCONSTRAINTEQUATIONCREATEFROMCOINCIDENTNODESCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you create your desired constraints
 */
class ccxConstraintEquationCreateFromCoincidentNodesCommand : public CubitCommand
{
public:
  ccxConstraintEquationCreateFromCoincidentNodesCommand();
  ~ccxConstraintEquationCreateFromCoincidentNodesCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTEQUATIONCREATEFROMCOINCIDENTNODESCOMMAND_HPP