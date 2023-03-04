#ifndef CCXSTEPBUCKLEMODIFYCOMMAND_HPP
#define CCXSTEPBUCKLEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Step modify command lets you modify your desired Steps
 */
class ccxStepBuckleModifyCommand : public CubitCommand
{
public:
  ccxStepBuckleModifyCommand();
  ~ccxStepBuckleModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSTEPBUCKLEMODIFYCOMMAND_HPP