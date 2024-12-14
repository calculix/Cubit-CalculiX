#ifndef CCXSTEPCOMPLEXFREQUENCYMODIFYCOMMAND_HPP
#define CCXSTEPCOMPLEXFREQUENCYMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepComplexFrequencyModifyCommand : public CubitCommand
{
public:
  ccxStepComplexFrequencyModifyCommand();
  ~ccxStepComplexFrequencyModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPCOMPLEXFREQUENCYMODIFYCOMMAND_HPP