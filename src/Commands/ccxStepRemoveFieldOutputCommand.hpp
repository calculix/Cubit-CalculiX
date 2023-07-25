#ifndef CCXSTEPREMOVEFIELDOUTPUTCOMMAND_HPP
#define CCXSTEPREMOVEFIELDOUTPUTCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step remove command lets you remove your desired outputs
 */
class ccxStepRemoveFieldOutputCommand : public CubitCommand
{
public:
  ccxStepRemoveFieldOutputCommand();
  ~ccxStepRemoveFieldOutputCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPREMOVEFIELDOUTPUTCOMMAND_HPP