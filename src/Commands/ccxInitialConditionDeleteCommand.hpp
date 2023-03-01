#ifndef CCXINITIALCONDITIONSDELETECOMMAND_HPP
#define CCXINITIALCONDITIONSDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Initial Condition delete command lets you delete your desired Initial Conditions
 */
class ccxInitialConditionDeleteCommand : public CubitCommand
{
public:
  ccxInitialConditionDeleteCommand();
  ~ccxInitialConditionDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXINITIALCONDITIONSDELETECOMMAND_HPP