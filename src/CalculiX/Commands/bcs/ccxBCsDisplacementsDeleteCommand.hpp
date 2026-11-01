#ifndef CCXBCSDISPLACEMENTSDELETECOMMAND_HPP
#define CCXBCSDISPLACEMENTSDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The displacement delete command
 */
class ccxBCsDisplacementsDeleteCommand : public CubitCommand
{
public:
  ccxBCsDisplacementsDeleteCommand();
  ~ccxBCsDisplacementsDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBCSDISPLACEMENTSDELETECOMMAND_HPP