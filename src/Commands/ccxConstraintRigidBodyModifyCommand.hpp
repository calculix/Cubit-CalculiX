#ifndef CCXCONSTRAINTRIGIDBODYMODIFYCOMMAND_HPP
#define CCXCONSTRAINTRIGIDBODYMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint modify command lets you modify your desired Constraint
 */
class ccxConstraintRigidBodyModifyCommand : public CubitCommand
{
public:
  ccxConstraintRigidBodyModifyCommand();
  ~ccxConstraintRigidBodyModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTRIGIDBODYMODIFYCOMMAND_HPP
