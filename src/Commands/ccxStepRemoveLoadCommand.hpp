#ifndef CCXSTEPREMOVELOADCOMMAND_HPP
#define CCXSTEPREMOVELOADCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step remove command lets you remove your desired loads
 */
class ccxStepRemoveLoadCommand : public CubitCommand
{
public:
  ccxStepRemoveLoadCommand();
  ~ccxStepRemoveLoadCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPREMOVELOADCOMMAND_HPP