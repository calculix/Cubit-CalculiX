#ifndef CCXSTEPUTILITYMODELCHANGESTEPCOMMAND_HPP
#define CCXSTEPUTILITYMODELCHANGESTEPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief creates steps with the model change keyword for an existing step
 */
class ccxStepUtilityModelChangeElementStepCommand : public CubitCommand
{
public:
  ccxStepUtilityModelChangeElementStepCommand();
  ~ccxStepUtilityModelChangeElementStepCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPUTILITYMODELCHANGESTEPCOMMAND_HPP