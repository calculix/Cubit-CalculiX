#ifndef CCXHBCREMOVEBCCOMMAND_HPP
#define CCXHBCREMOVEBCCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The hbc remove command lets you remove your desired bcs
 */
class ccxHBCRemoveBCCommand : public CubitCommand
{
public:
  ccxHBCRemoveBCCommand();
  ~ccxHBCRemoveBCCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHBCREMOVEBCCOMMAND_HPP