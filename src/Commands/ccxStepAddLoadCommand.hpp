#ifndef CCXSTEPADDLOADCOMMAND_HPP
#define CCXSTEPADDLOADCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step add command lets you add your desired loads
 */
class ccxStepAddLoadCommand : public CubitCommand
{
public:
  ccxStepAddLoadCommand();
  ~ccxStepAddLoadCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPADDLOADCOMMAND_HPP