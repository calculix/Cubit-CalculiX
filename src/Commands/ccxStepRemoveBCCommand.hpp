#ifndef CCXSTEPREMOVEBCCOMMAND_HPP
#define CCXSTEPREMOVEBCCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step remove command lets you remove your desired bcs
 */
class ccxStepRemoveBCCommand : public CubitCommand
{
public:
  ccxStepRemoveBCCommand();
  ~ccxStepRemoveBCCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPREMOVEBCCOMMAND_HPP