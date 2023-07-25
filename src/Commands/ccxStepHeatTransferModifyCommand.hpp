#ifndef CCXSTEPHEATTRANSFERMODIFYCOMMAND_HPP
#define CCXSTEPHEATTRANSFERMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepHeatTransferModifyCommand : public CubitCommand
{
public:
  ccxStepHeatTransferModifyCommand();
  ~ccxStepHeatTransferModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPHEATTRANSFERMODIFYCOMMAND_HPP