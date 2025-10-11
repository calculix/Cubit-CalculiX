#ifndef CCXSTEPUTILITYMODELCHANGEDUMMYSTEPCOMMAND_HPP
#define CCXSTEPUTILITYMODELCHANGEDUMMYSTEPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief adds dummy steps for the model change keyword
 */
class ccxStepUtilityModelChangeElementDummyStepCommand : public CubitCommand
{
public:
  ccxStepUtilityModelChangeElementDummyStepCommand();
  ~ccxStepUtilityModelChangeElementDummyStepCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPUTILITYMODELCHANGEDUMMYSTEPCOMMAND_HPP