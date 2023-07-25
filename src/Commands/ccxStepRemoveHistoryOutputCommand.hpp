#ifndef CCXSTEPREMOVEHISTORYOUTPUTCOMMAND_HPP
#define CCXSTEPREMOVEHISTORYOUTPUTCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step remove command lets you remove your desired outputs
 */
class ccxStepRemoveHistoryOutputCommand : public CubitCommand
{
public:
  ccxStepRemoveHistoryOutputCommand();
  ~ccxStepRemoveHistoryOutputCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPREMOVEHISTORYOUTPUTCOMMAND_HPP