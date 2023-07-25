#ifndef CCXCONSTRAINTTIEMODIFYCOMMAND_HPP
#define CCXCONSTRAINTTIEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you modify your desired constraints
 */
class ccxConstraintTieModifyCommand : public CubitCommand
{
public:
  ccxConstraintTieModifyCommand();
  ~ccxConstraintTieModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTTIEMODIFYCOMMAND_HPP
