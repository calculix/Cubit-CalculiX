#ifndef CCXSTEPADDFIELDOUTPUTCOMMAND_HPP
#define CCXSTEPADDFIELDOUTPUTCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The step add command lets you add your desired outputs
 */
class ccxStepAddFieldOutputCommand : public CubitCommand
{
public:
  ccxStepAddFieldOutputCommand();
  ~ccxStepAddFieldOutputCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPADDFIELDOUTPUTCOMMAND_HPP