#ifndef CCXSTEPSTATICMODIFYCOMMAND_HPP
#define CCXSTEPSTATICMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepStaticModifyCommand : public CubitCommand
{
public:
  ccxStepStaticModifyCommand();
  ~ccxStepStaticModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPSTATICMODIFYCOMMAND_HPP