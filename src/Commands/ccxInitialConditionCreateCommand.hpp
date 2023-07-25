#ifndef CCXINITIALCONDITIONCREATECOMMAND_HPP
#define CCXINITIALCONDITIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Initial Condition create command lets you create your desired Initial Conditions
 */
class ccxInitialConditionCreateCommand : public CubitCommand
{
public:
  ccxInitialConditionCreateCommand();
  ~ccxInitialConditionCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXINITIALCONDITIONCREATECOMMAND_HPP