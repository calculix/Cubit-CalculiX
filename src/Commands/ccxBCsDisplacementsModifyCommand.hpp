#ifndef CCXBCSDISPLACEMENTSMODIFYCOMMAND_HPP
#define CCXBCSDISPLACEMENTSMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The BCs modify command lets you modify your desired BCs
 */
class ccxBCsDisplacementsModifyCommand : public CubitCommand
{
public:
  ccxBCsDisplacementsModifyCommand();
  ~ccxBCsDisplacementsModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBCSDISPLACEMENTSMODIFYCOMMAND_HPP