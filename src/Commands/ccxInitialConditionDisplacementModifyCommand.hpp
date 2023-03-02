#ifndef CCXINITIALCONDITIONDISPLACEMENTMODIFYCOMMAND_HPP
#define CCXINITIALCONDITIONDISPLACEMENTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Initial Condition modify command lets you modify your desired Initial Conditions
 */
class ccxInitialConditionDisplacementModifyCommand : public CubitCommand
{
public:
  ccxInitialConditionDisplacementModifyCommand();
  ~ccxInitialConditionDisplacementModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXINITIALCONDITIONDISPLACEMENTMODIFYCOMMAND_HPP