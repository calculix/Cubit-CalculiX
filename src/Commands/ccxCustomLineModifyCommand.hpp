#ifndef CCXCUSTOMLINEMODIFYCOMMAND_HPP
#define CCXCUSTOMLINEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Job modify command lets you modify your desired Jobs
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