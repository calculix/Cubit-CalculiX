#ifndef CCXCONSTRAINTEQUATIONMODIFYCOMMAND_HPP
#define CCXCONSTRAINTEQUATIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you modify your desired constraints
 */
class ccxConstraintEquationModifyCommand : public CubitCommand
{
public:
  ccxConstraintEquationModifyCommand();
  ~ccxConstraintEquationModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTEQUATIONMODIFYCOMMAND_HPP