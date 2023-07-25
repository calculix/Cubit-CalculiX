#ifndef CCXSTEPADDBCCOMMAND_HPP
#define CCXSTEPADDBCCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step add command lets you add your desired bcs
 */
class ccxStepAddBCCommand : public CubitCommand
{
public:
  ccxStepAddBCCommand();
  ~ccxStepAddBCCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPADDBCCOMMAND_HPP