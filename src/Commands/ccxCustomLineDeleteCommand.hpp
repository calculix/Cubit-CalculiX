#ifndef CCXCUSTOMLINEDELETECOMMAND_HPP
#define CCXCUSTOMLINEDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The CustomLine delete command lets you delete your desired Custom Lines
 */
class ccxCustomLineDeleteCommand : public CubitCommand
{
public:
  ccxCustomLineDeleteCommand();
  ~ccxCustomLineDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCUSTOMLINEDELETECOMMAND_HPP