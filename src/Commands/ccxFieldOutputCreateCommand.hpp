#ifndef CCXFIELDOUTPUTCREATECOMMAND_HPP
#define CCXFIELDOUTPUTCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output create command lets you create your desired Outputs
 */
class ccxFieldOutputCreateCommand : public CubitCommand
{
public:
  ccxFieldOutputCreateCommand();
  ~ccxFieldOutputCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXFIELDOUTPUTCREATECOMMAND_HPP