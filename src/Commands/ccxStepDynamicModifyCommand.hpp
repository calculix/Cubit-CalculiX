#ifndef CCXSTEPDYNAMICMODIFYCOMMAND_HPP
#define CCXSTEPDYNAMICMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepDynamicModifyCommand : public CubitCommand
{
public:
  ccxStepDynamicModifyCommand();
  ~ccxStepDynamicModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPDYNAMICMODIFYCOMMAND_HPP