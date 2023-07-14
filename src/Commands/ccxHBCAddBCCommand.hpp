#ifndef CCXHBCADDBCCOMMAND_HPP
#define CCXHBCADDBCCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The hbc add command lets you add your desired bcs
 */
class ccxHBCAddBCCommand : public CubitCommand
{
public:
  ccxHBCAddBCCommand();
  ~ccxHBCAddBCCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHBCADDBCCOMMAND_HPP