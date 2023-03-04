#ifndef CCXSTEPPARAMETERMODIFYCOMMAND_HPP
#define CCXSTEPPARAMETERMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepParameterModifyCommand : public CubitCommand
{
public:
  ccxStepParameterModifyCommand();
  ~ccxStepParameterModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPPARAMETERMODIFYCOMMAND_HPP