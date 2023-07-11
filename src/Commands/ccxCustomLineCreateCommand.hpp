#ifndef CCXCUSTOMLINECREATECOMMAND_HPP
#define CCXCUSTOMLINECREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The CustomLine create command lets you create your desired Custom Lines
 */
class ccxCustomLineCreateCommand : public CubitCommand
{
public:
  ccxCustomLineCreateCommand();
  ~ccxCustomLineCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCUSTOMLINECREATECOMMAND_HPP