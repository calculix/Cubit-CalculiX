#ifndef CCXDRAWCOMMAND_HPP
#define CCXDRAWCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Draw command lets you draw the loads, bcs and orientations
 */
class ccxDrawCommand : public CubitCommand
{
public:
  ccxDrawCommand();
  ~ccxDrawCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXDRAWCOMMAND_HPP