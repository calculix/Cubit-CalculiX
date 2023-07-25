#ifndef CCXSTEPDELETECOMMAND_HPP
#define CCXSTEPDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step delete command lets you delete your desired Steps
 */
class ccxStepDeleteCommand : public CubitCommand
{
public:
  ccxStepDeleteCommand();
  ~ccxStepDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPDELETECOMMAND_HPP