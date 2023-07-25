#ifndef CCXFIELDOUTPUTELEMENTMODIFYCOMMAND_HPP
#define CCXFIELDOUTPUTELEMENTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxFieldOutputElementModifyCommand : public CubitCommand
{
public:
  ccxFieldOutputElementModifyCommand();
  ~ccxFieldOutputElementModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXFIELDOUTPUTELEMENTMODIFYCOMMAND_HPP