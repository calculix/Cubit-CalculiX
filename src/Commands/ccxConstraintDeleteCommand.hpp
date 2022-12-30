#ifndef CCXCONSTRAINTDELETECOMMAND_HPP
#define CCXCONSTRAINTDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The contraint delte command lets you delete your desired constraints
 */
class ccxConstraintDeleteCommand : public CubitCommand
{
public:
  ccxConstraintDeleteCommand();
  ~ccxConstraintDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTDELETECOMMAND_HPP
