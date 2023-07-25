#ifndef CCXINITIALCONDITIONTEMPERATUREMODIFYCOMMAND_HPP
#define CCXINITIALCONDITIONTEMPERATUREMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Initial Condition modify command lets you modify your desired Initial Conditions
 */
class ccxInitialConditionTemperatureModifyCommand : public CubitCommand
{
public:
  ccxInitialConditionTemperatureModifyCommand();
  ~ccxInitialConditionTemperatureModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXINITIALCONDITIONTEMPERATUREMODIFYCOMMAND_HPP