#ifndef CCXCONSTRAINTRIGIDBODY2CREATECOMMAND_HPP
#define CCXCONSTRAINTRIGIDBODY2CREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you create your desired constraints
 */
class ccxConstraintRigidBody2CreateCommand : public CubitCommand
{
public:
  ccxConstraintRigidBody2CreateCommand();
  ~ccxConstraintRigidBody2CreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTRIGIDBODY2CREATECOMMAND_HPP
