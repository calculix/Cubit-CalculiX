#ifndef CCXSTEPCREATECOMMAND_HPP
#define CCXSTEPCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step create command lets you create your desired steps
 */
class ccxStepCreateCommand : public CubitCommand
{
public:
  ccxStepCreateCommand();
  ~ccxStepCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPCREATECOMMAND_HPP