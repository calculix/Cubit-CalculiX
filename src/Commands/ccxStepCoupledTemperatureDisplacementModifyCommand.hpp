#ifndef CCXSTEPCOUPLEDTEMPERATUREDISPLACEMENTMODIFYCOMMAND_HPP
#define CCXSTEPCOUPLEDTEMPERATUREDISPLACEMENTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepCoupledTemperatureDisplacementModifyCommand : public CubitCommand
{
public:
  ccxStepCoupledTemperatureDisplacementModifyCommand();
  ~ccxStepCoupledTemperatureDisplacementModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPCOUPLEDTEMPERATUREDISPLACEMENTMODIFYCOMMAND_HPP