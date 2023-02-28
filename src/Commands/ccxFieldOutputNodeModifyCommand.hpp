#ifndef CCXFIELDOUTPUTNODEMODIFYCOMMAND_HPP
#define CCXFIELDOUTPUTNODEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxFieldOutputNodeModifyCommand : public CubitCommand
{
public:
  ccxFieldOutputNodeModifyCommand();
  ~ccxFieldOutputNodeModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXFIELDOUTPUTNODEMODIFYCOMMAND_HPP