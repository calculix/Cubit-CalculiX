#ifndef CCXFIELDOUTPUTCONTACTMODIFYCOMMAND_HPP
#define CCXFIELDOUTPUTCONTACTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxFieldOutputContactModifyCommand : public CubitCommand
{
public:
  ccxFieldOutputContactModifyCommand();
  ~ccxFieldOutputContactModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXFIELDOUTPUTCONTACTMODIFYCOMMAND_HPP