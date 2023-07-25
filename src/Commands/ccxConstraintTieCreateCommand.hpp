#ifndef CCXCONSTRAINTTIECREATECOMMAND_HPP
#define CCXCONSTRAINTTIECREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you create your desired constraints
 */
class ccxConstraintTieCreateCommand : public CubitCommand
{
public:
  ccxConstraintTieCreateCommand();
  ~ccxConstraintTieCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTTIECREATECOMMAND_HPP
