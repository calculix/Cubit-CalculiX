#ifndef CCXLOADSPRESSURESMODIFYCOMMAND_HPP
#define CCXLOADSPRESSURESMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads modify command lets you modify your desired Loads
 */
class ccxLoadsPressuresModifyCommand : public CubitCommand
{
public:
  ccxLoadsPressuresModifyCommand();
  ~ccxLoadsPressuresModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSPRESSURESMODIFYCOMMAND_HPP