#ifndef CCXLOADSPRESSURESDELETECOMMAND_HPP
#define CCXLOADSPRESSURESDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Pressure delete command lets you delete your desired Pressure Loads
 */
class ccxLoadsPressuresDeleteCommand : public CubitCommand
{
public:
  ccxLoadsPressuresDeleteCommand();
  ~ccxLoadsPressuresDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSPRESSURESDELETECOMMAND_HPP