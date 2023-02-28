#ifndef CCXFIELDOUTPUTDELETECOMMAND_HPP
#define CCXFIELDOUTPUTDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output delete command lets you delete your desired Outputs
 */
class ccxFieldOutputDeleteCommand : public CubitCommand
{
public:
  ccxFieldOutputDeleteCommand();
  ~ccxFieldOutputDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXFIELDOUTPUTDELETECOMMAND_HPP