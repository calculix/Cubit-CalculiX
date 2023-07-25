#ifndef CCXSTEPUNCOUPLEDTEMPERATUREDISPLACEMENTMODIFYCOMMAND_HPP
#define CCXSTEPUNCOUPLEDTEMPERATUREDISPLACEMENTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepUncoupledTemperatureDisplacementModifyCommand : public CubitCommand
{
public:
  ccxStepUncoupledTemperatureDisplacementModifyCommand();
  ~ccxStepUncoupledTemperatureDisplacementModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPUNCOUPLEDTEMPERATUREDISPLACEMENTMODIFYCOMMAND_HPP