#ifndef CCXCONSTRAINTEQUATIONCREATECOMMAND_HPP
#define CCXCONSTRAINTEQUATIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you create your desired constraints
 */
class ccxConstraintEquationCreateCommand : public CubitCommand
{
public:
  ccxConstraintEquationCreateCommand();
  ~ccxConstraintEquationCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTEQUATIONCREATECOMMAND_HPP