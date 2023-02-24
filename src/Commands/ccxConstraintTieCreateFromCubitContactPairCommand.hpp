#ifndef CCXCONSTRAINTTIECREATEFROMCUBITCONTACTPAIRCOMMAND_HPP
#define CCXCONSTRAINTTIECREATEFROMCUBITCONTACTPAIRCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint Tie create from cubit contact pairs command lets you convert Contact Pairs from cubit to ccx
 */
class ccxConstraintTieCreateFromCubitContactPairCommand : public CubitCommand
{
public:
  ccxConstraintTieCreateFromCubitContactPairCommand();
  ~ccxConstraintTieCreateFromCubitContactPairCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTTIECREATEFROMCUBITCONTACTPAIRCOMMAND_HPP
