#ifndef CCXCUSTOMLINEMODIFYCOMMAND_HPP
#define CCXCUSTOMLINEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The CustomLine modify command lets you modify your desired Custom Lines
 */
class ccxCustomLineModifyCommand : public CubitCommand
{
public:
  ccxCustomLineModifyCommand();
  ~ccxCustomLineModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCUSTOMLINEMODIFYCOMMAND_HPP