#ifndef CCXSTEPSTEADYSTATEDYNAMICSMODIFYCOMMAND_HPP
#define CCXSTEPSTEADYSTATEDYNAMICSMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepSteadyStateDynamicsModifyCommand : public CubitCommand
{
public:
  ccxStepSteadyStateDynamicsModifyCommand();
  ~ccxStepSteadyStateDynamicsModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPSTEADYSTATEDYNAMICSMODIFYCOMMAND_HPP