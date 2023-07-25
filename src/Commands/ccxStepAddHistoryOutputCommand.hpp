#ifndef CCXSTEPADDHISTORYOUTPUTCOMMAND_HPP
#define CCXSTEPADDHISTORYOUTPUTCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step add command lets you add your desired outputs
 */
class ccxStepAddHistoryOutputCommand : public CubitCommand
{
public:
  ccxStepAddHistoryOutputCommand();
  ~ccxStepAddHistoryOutputCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPADDHISTORYOUTPUTCOMMAND_HPP