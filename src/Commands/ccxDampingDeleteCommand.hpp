#ifndef CCXDAMPINGDELETECOMMAND_HPP
#define CCXDAMPINGDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Damping delete command lets you delete your desired Damping
 */
class ccxDampingDeleteCommand : public CubitCommand
{
public:
  ccxDampingDeleteCommand();
  ~ccxDampingDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXDAMPINGDELETECOMMAND_HPP