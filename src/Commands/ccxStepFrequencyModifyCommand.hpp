#ifndef CCXSTEPFREQUENCYMODIFYCOMMAND_HPP
#define CCXSTEPFREQUENCYMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepFrequencyModifyCommand : public CubitCommand
{
public:
  ccxStepFrequencyModifyCommand();
  ~ccxStepFrequencyModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPFREQUENCYMODIFYCOMMAND_HPP