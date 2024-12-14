#ifndef CCXSTEPMODALDYNAMICMODIFYCOMMAND_HPP
#define CCXSTEPMODALDYNAMICMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepModalDynamicModifyCommand : public CubitCommand
{
public:
  ccxStepModalDynamicModifyCommand();
  ~ccxStepModalDynamicModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPMODALDYNAMICMODIFYCOMMAND_HPP