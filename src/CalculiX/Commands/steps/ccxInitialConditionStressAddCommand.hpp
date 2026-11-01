#ifndef CCXINITIALCONDITIONSTRESSADDCOMMAND_HPP
#define CCXINITIALCONDITIONSTRESSADDCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Initial Condition Stress add command lets you add your desired Initial Conditions stress values
 */
class ccxInitialConditionStressAddCommand : public CubitCommand
{
public:
  ccxInitialConditionStressAddCommand();
  ~ccxInitialConditionStressAddCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXINITIALCONDITIONSTRESSADDCOMMAND_HPP