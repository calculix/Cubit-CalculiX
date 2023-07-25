#ifndef CCXCONSTRAINTRIGIDBODYCREATECOMMAND_HPP
#define CCXCONSTRAINTRIGIDBODYCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Constraint create command lets you create your desired constraints
 */
class ccxConstraintRigidBodyCreateCommand : public CubitCommand
{
public:
  ccxConstraintRigidBodyCreateCommand();
  ~ccxConstraintRigidBodyCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONSTRAINTRIGIDBODYCREATECOMMAND_HPP
